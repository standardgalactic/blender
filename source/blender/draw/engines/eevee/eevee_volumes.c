/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2016 Blender Foundation. */

/** \file
 * \ingroup draw_engine
 *
 * Volumetric effects rendering using frostbite approach.
 */

#include "DRW_render.h"

#include "BLI_listbase.h"
#include "BLI_rand.h"
#include "BLI_string_utils.h"

#include "DNA_fluid_types.h"
#include "DNA_object_force_types.h"
#include "DNA_volume_types.h"
#include "DNA_world_types.h"

#include "BKE_fluid.h"
#include "BKE_global.h"
#include "BKE_mesh.h"
#include "BKE_modifier.h"
#include "BKE_volume.h"
#include "BKE_volume_render.h"

#include "ED_screen.h"

#include "DEG_depsgraph_query.h"

#include "GPU_capabilities.h"
#include "GPU_material.h"
#include "GPU_texture.h"
#include "eevee_private.h"

static struct {
  GPUTexture *depth_src;

  GPUTexture *dummy_zero;
  GPUTexture *dummy_one;
  GPUTexture *dummy_flame;

  GPUTexture *dummy_scatter;
  GPUTexture *dummy_transmit;

  /* List of all fluid simulation / smoke domains rendered within this frame. */
  ListBase smoke_domains;
} e_data = {NULL}; /* Engine data */

static void eevee_create_textures_volumes(void)
{
  const float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  e_data.dummy_zero = DRW_texture_create_3d(1, 1, 1, GPU_RGBA8, DRW_TEX_WRAP, zero);

  const float one[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  e_data.dummy_one = DRW_texture_create_3d(1, 1, 1, GPU_RGBA8, DRW_TEX_WRAP, one);

  const float flame = 0.0f;
  e_data.dummy_flame = DRW_texture_create_3d(1, 1, 1, GPU_R8, DRW_TEX_WRAP, &flame);
}

static GPUTexture *eevee_volume_default_texture(eGPUVolumeDefaultValue default_value)
{
  switch (default_value) {
    case GPU_VOLUME_DEFAULT_0:
      return e_data.dummy_zero;
    case GPU_VOLUME_DEFAULT_1:
      return e_data.dummy_one;
  }

  return e_data.dummy_zero;
}

void EEVEE_volumes_set_jitter(EEVEE_ViewLayerData *sldata, uint current_sample)
{
  EEVEE_CommonUniformBuffer *common_data = &sldata->common_data;

  double ht_point[3];
  double ht_offset[3] = {0.0, 0.0};
  const uint ht_primes[3] = {3, 7, 2};

  BLI_halton_3d(ht_primes, ht_offset, current_sample, ht_point);

  common_data->vol_jitter[0] = (float)ht_point[0];
  common_data->vol_jitter[1] = (float)ht_point[1];
  common_data->vol_jitter[2] = (float)ht_point[2];
}

void EEVEE_volumes_init(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata)
{
  EEVEE_StorageList *stl = vedata->stl;
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_TextureList *txl = vedata->txl;
  EEVEE_EffectsInfo *effects = stl->effects;
  EEVEE_CommonUniformBuffer *common_data = &sldata->common_data;

  const DRWContextState *draw_ctx = DRW_context_state_get();
  const Scene *scene_eval = DEG_get_evaluated_scene(draw_ctx->depsgraph);

  const float *viewport_size = DRW_viewport_size_get();

  const int tile_size = scene_eval->eevee.volumetric_tile_size;

  /* Find Froxel Texture resolution. */
  int tex_size[3];

  tex_size[0] = (int)ceilf(fmaxf(1.0f, viewport_size[0] / (float)tile_size));
  tex_size[1] = (int)ceilf(fmaxf(1.0f, viewport_size[1] / (float)tile_size));
  tex_size[2] = max_ii(scene_eval->eevee.volumetric_samples, 1);

  common_data->vol_coord_scale[0] = viewport_size[0] / (float)(tile_size * tex_size[0]);
  common_data->vol_coord_scale[1] = viewport_size[1] / (float)(tile_size * tex_size[1]);
  common_data->vol_coord_scale[2] = 1.0f / viewport_size[0];
  common_data->vol_coord_scale[3] = 1.0f / viewport_size[1];

  /* TODO: compute snap to maxZBuffer for clustered rendering. */
  if ((common_data->vol_tex_size[0] != tex_size[0]) ||
      (common_data->vol_tex_size[1] != tex_size[1]) ||
      (common_data->vol_tex_size[2] != tex_size[2])) {
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_scattering);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_extinction);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_emission);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_phase);
    DRW_TEXTURE_FREE_SAFE(txl->volume_scatter);
    DRW_TEXTURE_FREE_SAFE(txl->volume_transmit);
    DRW_TEXTURE_FREE_SAFE(txl->volume_scatter_history);
    DRW_TEXTURE_FREE_SAFE(txl->volume_transmit_history);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_scat_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_integ_fb);
    copy_v3_v3_int(common_data->vol_tex_size, tex_size);

    common_data->vol_inv_tex_size[0] = 1.0f / (float)(tex_size[0]);
    common_data->vol_inv_tex_size[1] = 1.0f / (float)(tex_size[1]);
    common_data->vol_inv_tex_size[2] = 1.0f / (float)(tex_size[2]);
  }

  /* Like frostbite's paper, 5% blend of the new frame. */
  common_data->vol_history_alpha = (txl->volume_prop_scattering == NULL) ? 0.0f : 0.95f;

  /* Temporal Super sampling jitter */
  uint ht_primes[3] = {3, 7, 2};
  uint current_sample = 0;

  /* If TAA is in use do not use the history buffer. */
  bool do_taa = ((effects->enabled_effects & EFFECT_TAA) != 0);

  if (draw_ctx->evil_C != NULL) {
    struct wmWindowManager *wm = CTX_wm_manager(draw_ctx->evil_C);
    do_taa = do_taa && (ED_screen_animation_no_scrub(wm) == NULL);
  }

  if (do_taa) {
    common_data->vol_history_alpha = 0.0f;
    current_sample = effects->taa_current_sample - 1;
    effects->volume_current_sample = -1;
  }
  else if (DRW_state_is_image_render()) {
    const uint max_sample = (ht_primes[0] * ht_primes[1] * ht_primes[2]);
    current_sample = effects->volume_current_sample = (effects->volume_current_sample + 1) %
                                                      max_sample;
    if (current_sample != max_sample - 1) {
      DRW_viewport_request_redraw();
    }
  }

  EEVEE_volumes_set_jitter(sldata, current_sample);

  float integration_start = scene_eval->eevee.volumetric_start;
  float integration_end = scene_eval->eevee.volumetric_end;
  effects->volume_light_clamp = scene_eval->eevee.volumetric_light_clamp;
  common_data->vol_shadow_steps = (float)scene_eval->eevee.volumetric_shadow_samples;
  if ((scene_eval->eevee.flag & SCE_EEVEE_VOLUMETRIC_SHADOWS) == 0) {
    common_data->vol_shadow_steps = 0;
  }

  if (DRW_view_is_persp_get(NULL)) {
    float sample_distribution = scene_eval->eevee.volumetric_sample_distribution;
    sample_distribution = 4.0f * (max_ff(1.0f - sample_distribution, 1e-2f));

    const float clip_start = DRW_view_near_distance_get(NULL);
    /* Negate */
    float near = integration_start = min_ff(-integration_start, clip_start - 1e-4f);
    float far = integration_end = min_ff(-integration_end, near - 1e-4f);

    common_data->vol_depth_param[0] = (far - near * exp2(1.0f / sample_distribution)) /
                                      (far - near);
    common_data->vol_depth_param[1] = (1.0f - common_data->vol_depth_param[0]) / near;
    common_data->vol_depth_param[2] = sample_distribution;
  }
  else {
    const float clip_start = DRW_view_near_distance_get(NULL);
    const float clip_end = DRW_view_far_distance_get(NULL);
    integration_start = min_ff(integration_end, clip_start);
    integration_end = max_ff(-integration_end, clip_end);

    common_data->vol_depth_param[0] = integration_start;
    common_data->vol_depth_param[1] = integration_end;
    common_data->vol_depth_param[2] = 1.0f / (integration_end - integration_start);
  }

  /* Disable clamp if equal to 0. */
  if (effects->volume_light_clamp == 0.0) {
    effects->volume_light_clamp = FLT_MAX;
  }

  common_data->vol_use_lights = (scene_eval->eevee.flag & SCE_EEVEE_VOLUMETRIC_LIGHTS) != 0;
  common_data->vol_use_soft_shadows = (scene_eval->eevee.flag & SCE_EEVEE_SHADOW_SOFT) != 0;

  if (!e_data.dummy_scatter) {
    const float scatter[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    const float transmit[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    e_data.dummy_scatter = DRW_texture_create_3d(1, 1, 1, GPU_RGBA8, DRW_TEX_WRAP, scatter);
    e_data.dummy_transmit = DRW_texture_create_3d(1, 1, 1, GPU_RGBA8, DRW_TEX_WRAP, transmit);
  }
}

void EEVEE_volumes_cache_init(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata)
{
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_StorageList *stl = vedata->stl;
  EEVEE_EffectsInfo *effects = stl->effects;
  EEVEE_CommonUniformBuffer *common_data = &sldata->common_data;

  const DRWContextState *draw_ctx = DRW_context_state_get();
  Scene *scene = draw_ctx->scene;
  DRWShadingGroup *grp = NULL;

  /* Textures */
  if (!e_data.dummy_zero) {
    eevee_create_textures_volumes();
  }

  /* Quick breakdown of the Volumetric rendering:
   *
   * The rendering is separated in 4 stages:
   *
   * - Material Parameters : we collect volume properties of
   *   all participating media in the scene and store them in
   *   a 3D texture aligned with the 3D frustum.
   *   This is done in 2 passes, one that clear the texture
   *   and/or evaluate the world volumes, and the 2nd one that
   *   additively render object volumes.
   *
   * - Light Scattering : the volume properties then are sampled
   *   and light scattering is evaluated for each cell of the
   *   volume texture. Temporal super-sampling (if enabled) occurs here.
   *
   * - Volume Integration : the scattered light and extinction is
   *   integrated (accumulated) along the view-rays. The result is stored
   *   for every cell in another texture.
   *
   * - Full-screen Resolve : From the previous stage, we get two
   *   3D textures that contains integrated scattered light and extinction
   *   for "every" positions in the frustum. We only need to sample
   *   them and blend the scene color with those factors. This also
   *   work for alpha blended materials.
   */

  /* World pass is not additive as it also clear the buffer. */
  DRW_PASS_CREATE(psl->volumetric_world_ps, DRW_STATE_WRITE_COLOR);
  DRW_PASS_CREATE(psl->volumetric_objects_ps, DRW_STATE_WRITE_COLOR | DRW_STATE_BLEND_ADD);

  /* World Volumetric */
  struct World *wo = scene->world;
  if (wo != NULL && wo->use_nodes && wo->nodetree &&
      !LOOK_DEV_STUDIO_LIGHT_ENABLED(draw_ctx->v3d)) {
    struct GPUMaterial *mat = EEVEE_material_get(vedata, scene, NULL, wo, VAR_MAT_VOLUME);

    if (GPU_material_has_volume_output(mat)) {
      grp = DRW_shgroup_material_create(mat, psl->volumetric_world_ps);
    }

    if (grp) {
      DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
      /* TODO(fclem): remove those (need to clean the GLSL files). */
      DRW_shgroup_uniform_block(grp, "grid_block", sldata->grid_ubo);
      DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
      DRW_shgroup_uniform_block(grp, "planar_block", sldata->planar_ubo);
      DRW_shgroup_uniform_block(grp, "light_block", sldata->light_ubo);
      DRW_shgroup_uniform_block(grp, "shadow_block", sldata->shadow_ubo);
      DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);

      /* Fix principle volumetric not working with world materials. */
      ListBase gpu_grids = GPU_material_volume_grids(mat);
      LISTBASE_FOREACH (GPUMaterialVolumeGrid *, gpu_grid, &gpu_grids) {
        DRW_shgroup_uniform_texture(
            grp, gpu_grid->sampler_name, eevee_volume_default_texture(gpu_grid->default_value));
      }

      DRW_shgroup_call_procedural_triangles(grp, NULL, common_data->vol_tex_size[2]);

      effects->enabled_effects |= (EFFECT_VOLUMETRIC | EFFECT_POST_BUFFER);
    }
  }

  if (grp == NULL) {
    /* If no world or volume material is present just clear the buffer with this drawcall */
    grp = DRW_shgroup_create(EEVEE_shaders_volumes_clear_sh_get(), psl->volumetric_world_ps);
    DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
    DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
    DRW_shgroup_uniform_block(grp, "light_block", sldata->light_ubo);
    DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);

    DRW_shgroup_call_procedural_triangles(grp, NULL, common_data->vol_tex_size[2]);
  }
}

static bool eevee_volume_object_grids_init(Object *ob, ListBase *gpu_grids, DRWShadingGroup *grp)
{
  Volume *volume = ob->data;
  BKE_volume_load(volume, G.main);

  /* Test if we need to use multiple transforms. */
  DRWVolumeGrid *first_drw_grid = NULL;
  bool multiple_transforms = true;

  LISTBASE_FOREACH (GPUMaterialVolumeGrid *, gpu_grid, gpu_grids) {
    const VolumeGrid *volume_grid = BKE_volume_grid_find_for_read(volume, gpu_grid->name);
    DRWVolumeGrid *drw_grid = (volume_grid) ?
                                  DRW_volume_batch_cache_get_grid(volume, volume_grid) :
                                  NULL;

    if (drw_grid) {
      if (first_drw_grid == NULL) {
        first_drw_grid = drw_grid;
      }
      else if (drw_grid &&
               !equals_m4m4(drw_grid->object_to_texture, first_drw_grid->object_to_texture)) {
        multiple_transforms = true;
        break;
      }
    }
  }

  /* Bail out of no grids to render. */
  if (first_drw_grid == NULL) {
    return false;
  }

  /* Set transform matrix for the volume as a whole. This one is also used for
   * clipping so must map the entire bounding box to 0..1. */
  float bounds_to_object[4][4];

  if (multiple_transforms) {
    /* For multiple grids with different transform, we first transform from object space
     * to bounds, then for each individual grid from bounds to texture. */
    const BoundBox *bb = BKE_volume_boundbox_get(ob);
    float bb_size[3];
    sub_v3_v3v3(bb_size, bb->vec[6], bb->vec[0]);
    size_to_mat4(bounds_to_object, bb_size);
    copy_v3_v3(bounds_to_object[3], bb->vec[0]);

    invert_m4_m4(first_drw_grid->object_to_bounds, bounds_to_object);
    DRW_shgroup_uniform_mat4(grp, "volumeObjectToTexture", first_drw_grid->object_to_bounds);
  }
  else {
    /* All grid transforms are equal, we can transform to texture space immediately. */
    DRW_shgroup_uniform_mat4(grp, "volumeObjectToTexture", first_drw_grid->object_to_texture);
  }

  /* Don't use orco transform here, only matrix. */
  DRW_shgroup_uniform_vec3_copy(grp, "volumeOrcoLoc", (float[3]){0.5f, 0.5f, 0.5f});
  DRW_shgroup_uniform_vec3_copy(grp, "volumeOrcoSize", (float[3]){0.5f, 0.5f, 0.5f});

  /* Set density scale. */
  const float density_scale = BKE_volume_density_scale(volume, ob->obmat);
  DRW_shgroup_uniform_float_copy(grp, "volumeDensityScale", density_scale);

  /* Bind volume grid textures. */
  LISTBASE_FOREACH (GPUMaterialVolumeGrid *, gpu_grid, gpu_grids) {
    const VolumeGrid *volume_grid = BKE_volume_grid_find_for_read(volume, gpu_grid->name);
    DRWVolumeGrid *drw_grid = (volume_grid) ?
                                  DRW_volume_batch_cache_get_grid(volume, volume_grid) :
                                  NULL;

    /* Handle 3 cases here:
     * - Grid exists and texture was loaded -> use texture.
     * - Grid exists but has zero size or failed to load -> use zero.
     * - Grid does not exist -> use default value. */
    GPUTexture *grid_tex = (drw_grid)    ? drw_grid->texture :
                           (volume_grid) ? e_data.dummy_zero :
                                           eevee_volume_default_texture(gpu_grid->default_value);

    DRW_shgroup_uniform_texture(grp, gpu_grid->sampler_name, grid_tex);

    if (drw_grid && multiple_transforms) {
      /* Specify per-volume transform matrix that is applied after the
       * transform from object to bounds. */
      mul_m4_m4m4(drw_grid->bounds_to_texture, drw_grid->object_to_texture, bounds_to_object);
      DRW_shgroup_uniform_mat4(grp, gpu_grid->transform_name, drw_grid->bounds_to_texture);
    }
  }

  return true;
}

static bool eevee_volume_object_mesh_init(Scene *scene,
                                          Object *ob,
                                          ListBase *gpu_grids,
                                          DRWShadingGroup *grp)
{
  static const float white[3] = {1.0f, 1.0f, 1.0f};
  ModifierData *md = NULL;

  /* Smoke Simulation */
  if ((md = BKE_modifiers_findby_type(ob, eModifierType_Fluid)) &&
      (BKE_modifier_is_enabled(scene, md, eModifierMode_Realtime)) &&
      ((FluidModifierData *)md)->domain != NULL) {
    FluidModifierData *fmd = (FluidModifierData *)md;
    FluidDomainSettings *fds = fmd->domain;

    /* Don't try to show liquid domains here. */
    if (!fds->fluid || !(fds->type == FLUID_DOMAIN_TYPE_GAS)) {
      return false;
    }

    /* Don't show smoke before simulation starts, this could be made an option in the future. */
    /* (sebbas): Always show smoke for manta */
#if 0
    const DRWContextState *draw_ctx = DRW_context_state_get();
    const bool show_smoke = ((int)DEG_get_ctime(draw_ctx->depsgraph) >=
                             *fds->point_cache[0]->startframe);
#endif

    if (fds->fluid && (fds->type == FLUID_DOMAIN_TYPE_GAS) /* && show_smoke */) {
      DRW_smoke_ensure(fmd, fds->flags & FLUID_DOMAIN_USE_NOISE);
      BLI_addtail(&e_data.smoke_domains, BLI_genericNodeN(fmd));
    }

    LISTBASE_FOREACH (GPUMaterialVolumeGrid *, gpu_grid, gpu_grids) {
      if (STREQ(gpu_grid->name, "density")) {
        DRW_shgroup_uniform_texture_ref(
            grp, gpu_grid->sampler_name, fds->tex_density ? &fds->tex_density : &e_data.dummy_one);
      }
      else if (STREQ(gpu_grid->name, "color")) {
        DRW_shgroup_uniform_texture_ref(
            grp, gpu_grid->sampler_name, fds->tex_color ? &fds->tex_color : &e_data.dummy_one);
      }
      else if (STR_ELEM(gpu_grid->name, "flame", "temperature")) {
        DRW_shgroup_uniform_texture_ref(
            grp, gpu_grid->sampler_name, fds->tex_flame ? &fds->tex_flame : &e_data.dummy_flame);
      }
      else {
        DRW_shgroup_uniform_texture(
            grp, gpu_grid->sampler_name, eevee_volume_default_texture(gpu_grid->default_value));
      }
    }

    /* Constant Volume color. */
    bool use_constant_color = ((fds->active_fields & FLUID_DOMAIN_ACTIVE_COLORS) == 0 &&
                               (fds->active_fields & FLUID_DOMAIN_ACTIVE_COLOR_SET) != 0);

    DRW_shgroup_uniform_vec3(
        grp, "volumeColor", (use_constant_color) ? fds->active_color : white, 1);

    /* Output is such that 0..1 maps to 0..1000K */
    DRW_shgroup_uniform_vec2(grp, "volumeTemperature", &fds->flame_ignition, 1);
  }
  else {
    LISTBASE_FOREACH (GPUMaterialVolumeGrid *, gpu_grid, gpu_grids) {
      DRW_shgroup_uniform_texture(
          grp, gpu_grid->sampler_name, eevee_volume_default_texture(gpu_grid->default_value));
    }
  }

  /* Transform for mesh volumes. */
  static const float unit_mat[4][4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f, 0.0f},
                                       {0.0f, 0.0f, 1.0f, 0.0f},
                                       {0.0f, 0.0f, 0.0f, 1.0f}};
  float *texco_loc, *texco_size;
  BKE_mesh_texspace_get_reference((struct Mesh *)ob->data, NULL, &texco_loc, &texco_size);

  DRW_shgroup_uniform_mat4(grp, "volumeObjectToTexture", unit_mat);
  DRW_shgroup_uniform_vec3(grp, "volumeOrcoLoc", texco_loc, 1);
  DRW_shgroup_uniform_vec3(grp, "volumeOrcoSize", texco_size, 1);

  return true;
}

void EEVEE_volumes_cache_object_add(EEVEE_ViewLayerData *sldata,
                                    EEVEE_Data *vedata,
                                    Scene *scene,
                                    Object *ob)
{
  Material *ma = BKE_object_material_get_eval(ob, 1);

  if (ma == NULL) {
    if (ob->type == OB_VOLUME) {
      ma = BKE_material_default_volume();
    }
    else {
      return;
    }
  }

  float size[3];
  mat4_to_size(size, ob->obmat);
  /* Check if any of the axes have 0 length. (see T69070) */
  const float epsilon = 1e-8f;
  if ((size[0] < epsilon) || (size[1] < epsilon) || (size[2] < epsilon)) {
    return;
  }

  int mat_options = VAR_MAT_VOLUME | VAR_MAT_MESH;
  struct GPUMaterial *mat = EEVEE_material_get(vedata, scene, ma, NULL, mat_options);
  eGPUMaterialStatus status = GPU_material_status(mat);

  /* If shader failed to compile or is currently compiling. */
  if (status != GPU_MAT_SUCCESS) {
    return;
  }

  DRWShadingGroup *grp = DRW_shgroup_material_create(mat, vedata->psl->volumetric_objects_ps);

  /* TODO(fclem): remove those "unnecessary" UBOs */
  DRW_shgroup_uniform_block(grp, "planar_block", sldata->planar_ubo);
  DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
  DRW_shgroup_uniform_block(grp, "shadow_block", sldata->shadow_ubo);
  DRW_shgroup_uniform_block(grp, "light_block", sldata->light_ubo);
  DRW_shgroup_uniform_block(grp, "grid_block", sldata->grid_ubo);
  DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);

  DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);

  ListBase gpu_grids = GPU_material_volume_grids(mat);

  if (ob->type == OB_VOLUME) {
    if (!eevee_volume_object_grids_init(ob, &gpu_grids, grp)) {
      return;
    }
  }
  else {
    if (!eevee_volume_object_mesh_init(scene, ob, &gpu_grids, grp)) {
      return;
    }
  }

  /* TODO: Reduce to number of slices intersecting. */
  /* TODO: Preemptive culling. */
  DRW_shgroup_call_procedural_triangles(grp, ob, sldata->common_data.vol_tex_size[2]);

  vedata->stl->effects->enabled_effects |= (EFFECT_VOLUMETRIC | EFFECT_POST_BUFFER);
}

void EEVEE_volumes_cache_finish(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata)
{
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_TextureList *txl = vedata->txl;
  EEVEE_EffectsInfo *effects = vedata->stl->effects;
  LightCache *lcache = vedata->stl->g_data->light_cache;
  EEVEE_CommonUniformBuffer *common_data = &sldata->common_data;

  if ((effects->enabled_effects & EFFECT_VOLUMETRIC) != 0) {
    DRWShadingGroup *grp;
    struct GPUShader *sh;

    DRW_PASS_CREATE(psl->volumetric_scatter_ps, DRW_STATE_WRITE_COLOR);
    sh = (common_data->vol_use_lights) ? EEVEE_shaders_volumes_scatter_with_lights_sh_get() :
                                         EEVEE_shaders_volumes_scatter_sh_get();
    grp = DRW_shgroup_create(sh, psl->volumetric_scatter_ps);
    DRW_shgroup_uniform_texture_ref(grp, "irradianceGrid", &lcache->grid_tx.tex);
    DRW_shgroup_uniform_texture_ref(grp, "shadowCubeTexture", &sldata->shadow_cube_pool);
    DRW_shgroup_uniform_texture_ref(grp, "shadowCascadeTexture", &sldata->shadow_cascade_pool);
    DRW_shgroup_uniform_texture_ref(grp, "volumeScattering", &txl->volume_prop_scattering);
    DRW_shgroup_uniform_texture_ref(grp, "volumeExtinction", &txl->volume_prop_extinction);
    DRW_shgroup_uniform_texture_ref(grp, "volumeEmission", &txl->volume_prop_emission);
    DRW_shgroup_uniform_texture_ref(grp, "volumePhase", &txl->volume_prop_phase);
    DRW_shgroup_uniform_texture_ref(grp, "historyScattering", &txl->volume_scatter_history);
    DRW_shgroup_uniform_texture_ref(grp, "historyTransmittance", &txl->volume_transmit_history);
    DRW_shgroup_uniform_block(grp, "light_block", sldata->light_ubo);
    DRW_shgroup_uniform_block(grp, "shadow_block", sldata->shadow_ubo);
    DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
    DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
    DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);

    DRW_shgroup_call_procedural_triangles(grp, NULL, common_data->vol_tex_size[2]);

    DRW_PASS_CREATE(psl->volumetric_integration_ps, DRW_STATE_WRITE_COLOR);
    grp = DRW_shgroup_create(EEVEE_shaders_volumes_integration_sh_get(),
                             psl->volumetric_integration_ps);
    DRW_shgroup_uniform_texture_ref(grp, "volumeScattering", &txl->volume_scatter);
    DRW_shgroup_uniform_texture_ref(grp, "volumeExtinction", &txl->volume_transmit);
    DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
    DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
    DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);
    if (USE_VOLUME_OPTI) {
      DRW_shgroup_uniform_image_ref(grp, "finalScattering_img", &txl->volume_scatter_history);
      DRW_shgroup_uniform_image_ref(grp, "finalTransmittance_img", &txl->volume_transmit_history);
    }

    DRW_shgroup_call_procedural_triangles(
        grp, NULL, USE_VOLUME_OPTI ? 1 : common_data->vol_tex_size[2]);

    DRW_PASS_CREATE(psl->volumetric_resolve_ps, DRW_STATE_WRITE_COLOR | DRW_STATE_BLEND_CUSTOM);
    grp = DRW_shgroup_create(EEVEE_shaders_volumes_resolve_sh_get(false),
                             psl->volumetric_resolve_ps);
    DRW_shgroup_uniform_texture_ref(grp, "inScattering", &txl->volume_scatter);
    DRW_shgroup_uniform_texture_ref(grp, "inTransmittance", &txl->volume_transmit);
    DRW_shgroup_uniform_texture_ref(grp, "inSceneDepth", &e_data.depth_src);
    DRW_shgroup_uniform_block(grp, "light_block", sldata->light_ubo);
    DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
    DRW_shgroup_uniform_block(grp, "probe_block", sldata->probe_ubo);
    DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);
    DRW_shgroup_uniform_block(grp, "shadow_block", sldata->shadow_ubo);

    DRW_shgroup_call_procedural_triangles(grp, NULL, 1);
  }
}

void EEVEE_volumes_draw_init(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata)
{
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_TextureList *txl = vedata->txl;
  EEVEE_EffectsInfo *effects = vedata->stl->effects;
  EEVEE_CommonUniformBuffer *common_data = &sldata->common_data;

  if ((effects->enabled_effects & EFFECT_VOLUMETRIC) != 0) {
    int *tex_size = common_data->vol_tex_size;

    if (txl->volume_prop_scattering == NULL) {
      /* Volume properties: We evaluate all volumetric objects
       * and store their final properties into each froxel */
      txl->volume_prop_scattering = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
      txl->volume_prop_extinction = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
      txl->volume_prop_emission = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
      txl->volume_prop_phase = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_RG16F, DRW_TEX_FILTER, NULL);

      /* Volume scattering: We compute for each froxel the
       * Scattered light towards the view. We also resolve temporal
       * super sampling during this stage. */
      txl->volume_scatter = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
      txl->volume_transmit = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);

      /* Final integration: We compute for each froxel the
       * amount of scattered light and extinction coef at this
       * given depth. We use these textures as double buffer
       * for the volumetric history. */
      txl->volume_scatter_history = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
      txl->volume_transmit_history = DRW_texture_create_3d(
          tex_size[0], tex_size[1], tex_size[2], GPU_R11F_G11F_B10F, DRW_TEX_FILTER, NULL);
    }

    GPU_framebuffer_ensure_config(&fbl->volumetric_fb,
                                  {GPU_ATTACHMENT_NONE,
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_prop_scattering),
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_prop_extinction),
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_prop_emission),
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_prop_phase)});
    GPU_framebuffer_ensure_config(&fbl->volumetric_scat_fb,
                                  {GPU_ATTACHMENT_NONE,
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_scatter),
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_transmit)});
    GPU_framebuffer_ensure_config(&fbl->volumetric_integ_fb,
                                  {GPU_ATTACHMENT_NONE,
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_scatter_history),
                                   GPU_ATTACHMENT_TEXTURE(txl->volume_transmit_history)});
  }
  else {
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_scattering);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_extinction);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_emission);
    DRW_TEXTURE_FREE_SAFE(txl->volume_prop_phase);
    DRW_TEXTURE_FREE_SAFE(txl->volume_scatter);
    DRW_TEXTURE_FREE_SAFE(txl->volume_transmit);
    DRW_TEXTURE_FREE_SAFE(txl->volume_scatter_history);
    DRW_TEXTURE_FREE_SAFE(txl->volume_transmit_history);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_scat_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(fbl->volumetric_integ_fb);
  }

  effects->volume_scatter = e_data.dummy_scatter;
  effects->volume_transmit = e_data.dummy_transmit;
}

void EEVEE_volumes_compute(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata)
{
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_TextureList *txl = vedata->txl;
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_StorageList *stl = vedata->stl;
  EEVEE_EffectsInfo *effects = stl->effects;
  if ((effects->enabled_effects & EFFECT_VOLUMETRIC) != 0) {
    DRW_stats_group_start("Volumetrics");

    /* We sample the shadow-maps using shadow sampler. We need to enable Comparison mode.
     * TODO(fclem): avoid this by using sampler objects. */
    GPU_texture_compare_mode(sldata->shadow_cube_pool, true);
    GPU_texture_compare_mode(sldata->shadow_cascade_pool, true);

    GPU_framebuffer_bind(fbl->volumetric_fb);
    DRW_draw_pass(psl->volumetric_world_ps);
    DRW_draw_pass(psl->volumetric_objects_ps);

    GPU_framebuffer_bind(fbl->volumetric_scat_fb);
    DRW_draw_pass(psl->volumetric_scatter_ps);

    if (USE_VOLUME_OPTI) {
      /* Avoid feedback loop assert. */
      GPU_framebuffer_bind(fbl->volumetric_fb);
    }
    else {
      GPU_framebuffer_bind(fbl->volumetric_integ_fb);
    }

    DRW_draw_pass(psl->volumetric_integration_ps);

    SWAP(struct GPUFrameBuffer *, fbl->volumetric_scat_fb, fbl->volumetric_integ_fb);
    SWAP(GPUTexture *, txl->volume_scatter, txl->volume_scatter_history);
    SWAP(GPUTexture *, txl->volume_transmit, txl->volume_transmit_history);

    effects->volume_scatter = txl->volume_scatter;
    effects->volume_transmit = txl->volume_transmit;

    /* Restore */
    GPU_framebuffer_bind(fbl->main_fb);

    DRW_stats_group_end();
  }
}

void EEVEE_volumes_resolve(EEVEE_ViewLayerData *UNUSED(sldata), EEVEE_Data *vedata)
{
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_StorageList *stl = vedata->stl;
  EEVEE_EffectsInfo *effects = stl->effects;

  if ((effects->enabled_effects & EFFECT_VOLUMETRIC) != 0) {
    DefaultTextureList *dtxl = DRW_viewport_texture_list_get();
    e_data.depth_src = dtxl->depth;

    if (USE_VOLUME_OPTI) {
      GPU_memory_barrier(GPU_BARRIER_TEXTURE_FETCH);
    }

    /* Apply for opaque geometry. */
    GPU_framebuffer_bind(fbl->main_color_fb);
    DRW_draw_pass(psl->volumetric_resolve_ps);

    /* Restore. */
    GPU_framebuffer_bind(fbl->main_fb);
  }
}

void EEVEE_volumes_free_smoke_textures(void)
{
  /* Free Smoke Textures after rendering */
  LISTBASE_FOREACH (LinkData *, link, &e_data.smoke_domains) {
    FluidModifierData *fmd = (FluidModifierData *)link->data;
    DRW_smoke_free(fmd);
  }
  BLI_freelistN(&e_data.smoke_domains);
}

void EEVEE_volumes_free(void)
{
  DRW_TEXTURE_FREE_SAFE(e_data.dummy_scatter);
  DRW_TEXTURE_FREE_SAFE(e_data.dummy_transmit);

  DRW_TEXTURE_FREE_SAFE(e_data.dummy_zero);
  DRW_TEXTURE_FREE_SAFE(e_data.dummy_one);
  DRW_TEXTURE_FREE_SAFE(e_data.dummy_flame);
}

/* -------------------------------------------------------------------- */
/** \name Render Passes
 * \{ */

void EEVEE_volumes_output_init(EEVEE_ViewLayerData *sldata, EEVEE_Data *vedata, uint tot_samples)
{
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_TextureList *txl = vedata->txl;
  EEVEE_StorageList *stl = vedata->stl;
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_EffectsInfo *effects = stl->effects;

  /* Create FrameBuffer. */

  /* Should be enough precision for many samples. */
  const eGPUTextureFormat texture_format_accum = (tot_samples > 128) ? GPU_RGBA32F : GPU_RGBA16F;
  DRW_texture_ensure_fullscreen_2d(&txl->volume_scatter_accum, texture_format_accum, 0);
  DRW_texture_ensure_fullscreen_2d(&txl->volume_transmittance_accum, texture_format_accum, 0);

  GPU_framebuffer_ensure_config(&fbl->volumetric_accum_fb,
                                {GPU_ATTACHMENT_NONE,
                                 GPU_ATTACHMENT_TEXTURE(txl->volume_scatter_accum),
                                 GPU_ATTACHMENT_TEXTURE(txl->volume_transmittance_accum)});

  /* Create Pass and shgroup. */
  DRW_PASS_CREATE(psl->volumetric_accum_ps, DRW_STATE_WRITE_COLOR | DRW_STATE_BLEND_ADD_FULL);
  DRWShadingGroup *grp = NULL;
  if ((effects->enabled_effects & EFFECT_VOLUMETRIC) != 0) {
    grp = DRW_shgroup_create(EEVEE_shaders_volumes_resolve_sh_get(true), psl->volumetric_accum_ps);
    DRW_shgroup_uniform_texture_ref(grp, "inScattering", &txl->volume_scatter);
    DRW_shgroup_uniform_texture_ref(grp, "inTransmittance", &txl->volume_transmit);
    DRW_shgroup_uniform_texture_ref(grp, "inSceneDepth", &e_data.depth_src);
    DRW_shgroup_uniform_block(grp, "common_block", sldata->common_ubo);
    DRW_shgroup_uniform_block(grp, "renderpass_block", sldata->renderpass_ubo.combined);
  }
  else {
    /* There is no volumetrics in the scene. Use a shader to fill the accum textures with a default
     * value. */
    grp = DRW_shgroup_create(EEVEE_shaders_volumes_accum_sh_get(), psl->volumetric_accum_ps);
  }
  DRW_shgroup_call(grp, DRW_cache_fullscreen_quad_get(), NULL);
}

void EEVEE_volumes_output_accumulate(EEVEE_ViewLayerData *UNUSED(sldata), EEVEE_Data *vedata)
{
  EEVEE_FramebufferList *fbl = vedata->fbl;
  EEVEE_PassList *psl = vedata->psl;
  EEVEE_EffectsInfo *effects = vedata->stl->effects;

  if (fbl->volumetric_accum_fb != NULL) {
    /* Accum pass */
    GPU_framebuffer_bind(fbl->volumetric_accum_fb);

    /* Clear texture. */
    if (effects->taa_current_sample == 1) {
      const float clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};
      GPU_framebuffer_clear_color(fbl->volumetric_accum_fb, clear);
    }

    DRW_draw_pass(psl->volumetric_accum_ps);

    /* Restore */
    GPU_framebuffer_bind(fbl->main_fb);
  }
}

/** \} */
