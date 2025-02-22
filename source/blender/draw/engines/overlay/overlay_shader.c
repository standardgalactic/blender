/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2019 Blender Foundation. */

/** \file
 * \ingroup draw_engine
 */

#include "DRW_render.h"

#include "GPU_shader.h"

#include "UI_resources.h"

#include "overlay_private.h"

extern char datatoc_antialiasing_frag_glsl[];
extern char datatoc_antialiasing_vert_glsl[];
extern char datatoc_armature_dof_vert_glsl[];
extern char datatoc_armature_dof_solid_frag_glsl[];
extern char datatoc_armature_envelope_distance_frag_glsl[];
extern char datatoc_armature_envelope_outline_vert_glsl[];
extern char datatoc_armature_envelope_solid_frag_glsl[];
extern char datatoc_armature_envelope_solid_vert_glsl[];
extern char datatoc_armature_shape_outline_geom_glsl[];
extern char datatoc_armature_shape_outline_vert_glsl[];
extern char datatoc_armature_shape_solid_frag_glsl[];
extern char datatoc_armature_shape_solid_vert_glsl[];
extern char datatoc_armature_shape_wire_vert_glsl[];
extern char datatoc_armature_sphere_outline_vert_glsl[];
extern char datatoc_armature_sphere_solid_frag_glsl[];
extern char datatoc_armature_sphere_solid_vert_glsl[];
extern char datatoc_armature_stick_frag_glsl[];
extern char datatoc_armature_stick_vert_glsl[];
extern char datatoc_armature_wire_frag_glsl[];
extern char datatoc_armature_wire_vert_glsl[];
extern char datatoc_background_frag_glsl[];
extern char datatoc_clipbound_vert_glsl[];
extern char datatoc_depth_only_vert_glsl[];
extern char datatoc_edit_curve_handle_geom_glsl[];
extern char datatoc_edit_curve_handle_vert_glsl[];
extern char datatoc_edit_curve_point_vert_glsl[];
extern char datatoc_edit_curve_wire_vert_glsl[];
extern char datatoc_edit_gpencil_canvas_vert_glsl[];
extern char datatoc_edit_gpencil_guide_vert_glsl[];
extern char datatoc_edit_gpencil_vert_glsl[];
extern char datatoc_edit_lattice_point_vert_glsl[];
extern char datatoc_edit_lattice_wire_vert_glsl[];
extern char datatoc_edit_mesh_common_lib_glsl[];
extern char datatoc_edit_mesh_frag_glsl[];
extern char datatoc_edit_mesh_geom_glsl[];
extern char datatoc_edit_mesh_vert_glsl[];
extern char datatoc_edit_mesh_normal_vert_glsl[];
extern char datatoc_edit_mesh_skin_root_vert_glsl[];
extern char datatoc_edit_mesh_analysis_vert_glsl[];
extern char datatoc_edit_mesh_analysis_frag_glsl[];
extern char datatoc_edit_particle_strand_vert_glsl[];
extern char datatoc_edit_particle_point_vert_glsl[];
extern char datatoc_edit_uv_verts_vert_glsl[];
extern char datatoc_edit_uv_verts_frag_glsl[];
extern char datatoc_edit_uv_edges_vert_glsl[];
extern char datatoc_edit_uv_edges_geom_glsl[];
extern char datatoc_edit_uv_edges_frag_glsl[];
extern char datatoc_edit_uv_faces_vert_glsl[];
extern char datatoc_edit_uv_face_dots_vert_glsl[];
extern char datatoc_edit_uv_stretching_vert_glsl[];
extern char datatoc_edit_uv_image_vert_glsl[];
extern char datatoc_edit_uv_tiled_image_borders_vert_glsl[];
extern char datatoc_extra_frag_glsl[];
extern char datatoc_extra_vert_glsl[];
extern char datatoc_extra_groundline_vert_glsl[];
extern char datatoc_extra_lightprobe_grid_vert_glsl[];
extern char datatoc_extra_loose_point_vert_glsl[];
extern char datatoc_extra_loose_point_frag_glsl[];
extern char datatoc_extra_point_vert_glsl[];
extern char datatoc_extra_wire_frag_glsl[];
extern char datatoc_extra_wire_vert_glsl[];
extern char datatoc_facing_frag_glsl[];
extern char datatoc_facing_vert_glsl[];
extern char datatoc_grid_background_frag_glsl[];
extern char datatoc_grid_frag_glsl[];
extern char datatoc_grid_vert_glsl[];
extern char datatoc_image_frag_glsl[];
extern char datatoc_edit_uv_image_mask_frag_glsl[];
extern char datatoc_image_vert_glsl[];
extern char datatoc_motion_path_line_vert_glsl[];
extern char datatoc_motion_path_line_geom_glsl[];
extern char datatoc_motion_path_point_vert_glsl[];
extern char datatoc_outline_detect_frag_glsl[];
extern char datatoc_outline_prepass_frag_glsl[];
extern char datatoc_outline_prepass_geom_glsl[];
extern char datatoc_outline_prepass_vert_glsl[];
extern char datatoc_paint_face_vert_glsl[];
extern char datatoc_paint_point_vert_glsl[];
extern char datatoc_paint_texture_frag_glsl[];
extern char datatoc_paint_texture_vert_glsl[];
extern char datatoc_paint_vertcol_frag_glsl[];
extern char datatoc_paint_vertcol_vert_glsl[];
extern char datatoc_paint_weight_frag_glsl[];
extern char datatoc_paint_weight_vert_glsl[];
extern char datatoc_paint_wire_vert_glsl[];
extern char datatoc_particle_vert_glsl[];
extern char datatoc_particle_frag_glsl[];
extern char datatoc_sculpt_mask_vert_glsl[];
extern char datatoc_sculpt_mask_frag_glsl[];
extern char datatoc_volume_velocity_vert_glsl[];
extern char datatoc_volume_gridlines_vert_glsl[];
extern char datatoc_wireframe_vert_glsl[];
extern char datatoc_wireframe_frag_glsl[];
extern char datatoc_xray_fade_frag_glsl[];

extern char datatoc_gpu_shader_depth_only_frag_glsl[];
extern char datatoc_gpu_shader_point_varying_color_frag_glsl[];
extern char datatoc_gpu_shader_3D_smooth_color_frag_glsl[];
extern char datatoc_gpu_shader_2D_smooth_color_frag_glsl[];
extern char datatoc_gpu_shader_uniform_color_frag_glsl[];
extern char datatoc_gpu_shader_flat_color_frag_glsl[];
extern char datatoc_gpu_shader_point_varying_color_varying_outline_aa_frag_glsl[];
extern char datatoc_gpu_shader_common_obinfos_lib_glsl[];

extern char datatoc_gpencil_common_lib_glsl[];

extern char datatoc_common_overlay_lib_glsl[];
extern char datatoc_common_colormanagement_lib_glsl[];
extern char datatoc_common_fullscreen_vert_glsl[];
extern char datatoc_common_fxaa_lib_glsl[];
extern char datatoc_common_smaa_lib_glsl[];
extern char datatoc_common_globals_lib_glsl[];
extern char datatoc_common_pointcloud_lib_glsl[];
extern char datatoc_common_view_lib_glsl[];

typedef struct OVERLAY_Shaders {
  GPUShader *antialiasing;
  GPUShader *armature_dof_wire;
  GPUShader *armature_dof_solid;
  GPUShader *armature_envelope_outline;
  GPUShader *armature_envelope_solid;
  GPUShader *armature_shape_outline;
  GPUShader *armature_shape_solid;
  GPUShader *armature_shape_wire;
  GPUShader *armature_sphere_outline;
  GPUShader *armature_sphere_solid;
  GPUShader *armature_stick;
  GPUShader *armature_wire;
  GPUShader *background;
  GPUShader *clipbound;
  GPUShader *depth_only;
  GPUShader *edit_curve_handle;
  GPUShader *edit_curve_point;
  GPUShader *edit_curve_wire;
  GPUShader *edit_gpencil_guide_point;
  GPUShader *edit_gpencil_point;
  GPUShader *edit_gpencil_wire;
  GPUShader *edit_lattice_point;
  GPUShader *edit_lattice_wire;
  GPUShader *edit_mesh_vert;
  GPUShader *edit_mesh_edge;
  GPUShader *edit_mesh_edge_flat;
  GPUShader *edit_mesh_face;
  GPUShader *edit_mesh_facedot;
  GPUShader *edit_mesh_skin_root;
  GPUShader *edit_mesh_vnormals;
  GPUShader *edit_mesh_normals;
  GPUShader *edit_mesh_fnormals;
  GPUShader *edit_mesh_analysis;
  GPUShader *edit_particle_strand;
  GPUShader *edit_particle_point;
  GPUShader *edit_uv_verts;
  GPUShader *edit_uv_faces;
  GPUShader *edit_uv_edges;
  GPUShader *edit_uv_edges_for_edge_select;
  GPUShader *edit_uv_face_dots;
  GPUShader *edit_uv_stretching_angle;
  GPUShader *edit_uv_stretching_area;
  GPUShader *edit_uv_tiled_image_borders;
  GPUShader *edit_uv_stencil_image;
  GPUShader *edit_uv_mask_image;
  GPUShader *extra;
  GPUShader *extra_select;
  GPUShader *extra_groundline;
  GPUShader *extra_wire[2];
  GPUShader *extra_wire_select;
  GPUShader *extra_point;
  GPUShader *extra_lightprobe_grid;
  GPUShader *extra_loose_point;
  GPUShader *facing;
  GPUShader *gpencil_canvas;
  GPUShader *grid;
  GPUShader *grid_background;
  GPUShader *grid_image;
  GPUShader *image;
  GPUShader *motion_path_line;
  GPUShader *motion_path_vert;
  GPUShader *outline_prepass;
  GPUShader *outline_prepass_gpencil;
  GPUShader *outline_prepass_pointcloud;
  GPUShader *outline_prepass_wire;
  GPUShader *outline_detect;
  GPUShader *paint_face;
  GPUShader *paint_point;
  GPUShader *paint_texture;
  GPUShader *paint_vertcol;
  GPUShader *paint_weight[2];
  GPUShader *paint_wire;
  GPUShader *particle_dot;
  GPUShader *particle_shape;
  GPUShader *pointcloud_dot;
  GPUShader *sculpt_mask;
  GPUShader *uniform_color;
  GPUShader *volume_velocity_needle_sh;
  GPUShader *volume_velocity_mac_sh;
  GPUShader *volume_velocity_sh;
  GPUShader *volume_gridlines_sh;
  GPUShader *volume_gridlines_flags_sh;
  GPUShader *volume_gridlines_range_sh;
  GPUShader *wireframe_select;
  GPUShader *wireframe[2];
  GPUShader *xray_fade;
} OVERLAY_Shaders;

static struct {
  OVERLAY_Shaders sh_data[GPU_SHADER_CFG_LEN];
  DRWShaderLibrary *lib;
} e_data = {{{NULL}}};

void OVERLAY_shader_library_ensure(void)
{
  if (e_data.lib == NULL) {
    e_data.lib = DRW_shader_library_create();
    /* NOTE: These need to be ordered by dependencies. */
    DRW_SHADER_LIB_ADD(e_data.lib, common_globals_lib);
    DRW_SHADER_LIB_ADD(e_data.lib, common_overlay_lib);
    DRW_SHADER_LIB_ADD(e_data.lib, common_colormanagement_lib);
    DRW_SHADER_LIB_ADD(e_data.lib, common_view_lib);
  }
}

GPUShader *OVERLAY_shader_antialiasing(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->antialiasing) {
    sh_data->antialiasing = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_antialiasing_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_antialiasing_frag_glsl,
                                 NULL},
        .defs = (const char *[]){NULL},
    });
  }
  return sh_data->antialiasing;
}

GPUShader *OVERLAY_shader_background(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->background) {
    sh_data->background = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_fullscreen_vert_glsl, NULL},
        .frag =
            (const char *[]){datatoc_common_globals_lib_glsl, datatoc_background_frag_glsl, NULL},
    });
  }
  return sh_data->background;
}

GPUShader *OVERLAY_shader_clipbound(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[0];
  if (!sh_data->clipbound) {
    sh_data->clipbound = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_view_lib_glsl, datatoc_clipbound_vert_glsl, NULL},
        .frag = (const char *[]){datatoc_gpu_shader_uniform_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->clipbound;
}

GPUShader *OVERLAY_shader_depth_only(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->depth_only) {
    sh_data->depth_only = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_depth_only_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_depth_only_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->depth_only;
}

GPUShader *OVERLAY_shader_edit_mesh_vert(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_vert) {
    sh_data->edit_mesh_vert = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_vert_clipped" : "overlay_edit_mesh_vert");
  }
  return sh_data->edit_mesh_vert;
}

GPUShader *OVERLAY_shader_edit_mesh_edge(bool use_flat_interp)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  GPUShader **sh = use_flat_interp ? &sh_data->edit_mesh_edge_flat : &sh_data->edit_mesh_edge;
  if (*sh == NULL) {
    *sh = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ?
            (use_flat_interp ? "overlay_edit_mesh_edge_flat_clipped" :
                               "overlay_edit_mesh_edge_clipped") :
            (use_flat_interp ? "overlay_edit_mesh_edge_flat" : "overlay_edit_mesh_edge"));
  }
  return *sh;
}

GPUShader *OVERLAY_shader_armature_sphere(bool use_outline)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (use_outline && !sh_data->armature_sphere_outline) {
    sh_data->armature_sphere_outline = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_sphere_outline_clipped" :
                           "overlay_armature_sphere_outline");
  }
  else if (!sh_data->armature_sphere_solid) {
    sh_data->armature_sphere_solid = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_sphere_solid_clipped" :
                           "overlay_armature_sphere_solid");
  }
  return use_outline ? sh_data->armature_sphere_outline : sh_data->armature_sphere_solid;
}

GPUShader *OVERLAY_shader_armature_shape(bool use_outline)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (use_outline && !sh_data->armature_shape_outline) {
    sh_data->armature_shape_outline = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_shape_outline_clipped" :
                           "overlay_armature_shape_outline");
  }
  else if (!sh_data->armature_shape_solid) {
    sh_data->armature_shape_solid = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_shape_solid_clipped" :
                           "overlay_armature_shape_solid");
  }
  return use_outline ? sh_data->armature_shape_outline : sh_data->armature_shape_solid;
}

GPUShader *OVERLAY_shader_armature_shape_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->armature_shape_wire) {
    sh_data->armature_shape_wire = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_shape_wire_clipped" : "overlay_armature_shape_wire");
  }
  return sh_data->armature_shape_wire;
}

GPUShader *OVERLAY_shader_armature_envelope(bool use_outline)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (use_outline && !sh_data->armature_envelope_outline) {
    sh_data->armature_envelope_outline = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_envelope_outline_clipped" :
                           "overlay_armature_envelope_outline");
  }
  else if (!sh_data->armature_envelope_solid) {
    sh_data->armature_envelope_solid = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_envelope_solid_clipped" :
                           "overlay_armature_envelope_solid");
  }
  return use_outline ? sh_data->armature_envelope_outline : sh_data->armature_envelope_solid;
}

GPUShader *OVERLAY_shader_armature_stick(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->armature_stick) {
    sh_data->armature_stick = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_stick_clipped" : "overlay_armature_stick");
  }
  return sh_data->armature_stick;
}

GPUShader *OVERLAY_shader_armature_degrees_of_freedom_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->armature_dof_wire) {
    sh_data->armature_dof_wire = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_dof_wire_clipped" : "overlay_armature_dof_wire");
  }
  return sh_data->armature_dof_wire;
}

GPUShader *OVERLAY_shader_armature_degrees_of_freedom_solid(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->armature_dof_solid) {
    sh_data->armature_dof_solid = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_dof_solid_clipped" : "overlay_armature_dof_solid");
  }
  return sh_data->armature_dof_solid;
}

GPUShader *OVERLAY_shader_armature_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->armature_wire) {
    sh_data->armature_wire = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_armature_wire_clipped" : "overlay_armature_wire");
  }
  return sh_data->armature_wire;
}

GPUShader *OVERLAY_shader_edit_curve_handle(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_curve_handle) {
    sh_data->edit_curve_handle = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_curve_handle_vert_glsl,
                                 NULL},
        .geom = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_curve_handle_geom_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_3D_smooth_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_curve_handle;
}

GPUShader *OVERLAY_shader_edit_curve_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_curve_point) {
    sh_data->edit_curve_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_curve_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_curve_point;
}

GPUShader *OVERLAY_shader_edit_curve_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_curve_wire) {
    sh_data->edit_curve_wire = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_curve_wire_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_flat_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define IN_PLACE_INSTANCES\n", NULL},
    });
  }
  return sh_data->edit_curve_wire;
}

GPUShader *OVERLAY_shader_edit_gpencil_guide_point(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_gpencil_guide_point) {
    sh_data->edit_gpencil_guide_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_edit_gpencil_guide_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
    });
  }
  return sh_data->edit_gpencil_guide_point;
}

GPUShader *OVERLAY_shader_edit_gpencil_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_gpencil_point) {
    sh_data->edit_gpencil_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_gpencil_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define USE_POINTS\n", NULL},
    });
  }
  return sh_data->edit_gpencil_point;
}

GPUShader *OVERLAY_shader_edit_gpencil_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_gpencil_wire) {
    sh_data->edit_gpencil_wire = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_gpencil_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_3D_smooth_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_gpencil_wire;
}

GPUShader *OVERLAY_shader_edit_lattice_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_lattice_point) {
    sh_data->edit_lattice_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_lattice_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_lattice_point;
}

GPUShader *OVERLAY_shader_edit_lattice_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_lattice_wire) {
    sh_data->edit_lattice_wire = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_lattice_wire_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_3D_smooth_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_lattice_wire;
}

GPUShader *OVERLAY_shader_edit_mesh_face(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_face) {
    sh_data->edit_mesh_face = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_face_clipped" : "overlay_edit_mesh_face");
  }
  return sh_data->edit_mesh_face;
}

GPUShader *OVERLAY_shader_edit_mesh_facedot(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_facedot) {
    sh_data->edit_mesh_facedot = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_facedot_clipped" : "overlay_edit_mesh_facedot");
  }
  return sh_data->edit_mesh_facedot;
}

GPUShader *OVERLAY_shader_edit_mesh_normal(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_normals) {
    sh_data->edit_mesh_normals = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_normal_clipped" : "overlay_edit_mesh_normal");
  }
  return sh_data->edit_mesh_normals;
}

GPUShader *OVERLAY_shader_edit_mesh_analysis(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_analysis) {
    sh_data->edit_mesh_analysis = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_analysis_clipped" : "overlay_edit_mesh_analysis");
  }
  return sh_data->edit_mesh_analysis;
}

GPUShader *OVERLAY_shader_edit_mesh_skin_root(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_mesh_skin_root) {
    sh_data->edit_mesh_skin_root = GPU_shader_create_from_info_name(
        draw_ctx->sh_cfg ? "overlay_edit_mesh_skin_root_clipped" : "overlay_edit_mesh_skin_root");
  }
  return sh_data->edit_mesh_skin_root;
}

GPUShader *OVERLAY_shader_edit_particle_strand(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_particle_strand) {
    sh_data->edit_particle_strand = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_particle_strand_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_3D_smooth_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_particle_strand;
}

GPUShader *OVERLAY_shader_edit_particle_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->edit_particle_point) {
    sh_data->edit_particle_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_particle_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->edit_particle_point;
}

GPUShader *OVERLAY_shader_extra(bool is_select)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  GPUShader **sh = (is_select) ? &sh_data->extra_select : &sh_data->extra;
  if (!*sh) {
    *sh = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_extra_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl, datatoc_extra_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, (is_select) ? "#define SELECT_EDGES\n" : NULL, NULL},
    });
  }
  return *sh;
}

GPUShader *OVERLAY_shader_extra_grid(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->extra_lightprobe_grid) {
    sh_data->extra_lightprobe_grid = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_extra_lightprobe_grid_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->extra_lightprobe_grid;
}

GPUShader *OVERLAY_shader_extra_groundline(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->extra_groundline) {
    sh_data->extra_groundline = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_extra_groundline_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl, datatoc_extra_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->extra_groundline;
}

GPUShader *OVERLAY_shader_extra_wire(bool use_object, bool is_select)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  GPUShader **sh = (is_select) ? &sh_data->extra_wire_select : &sh_data->extra_wire[use_object];
  if (!*sh) {
    char colorids[1024];
    /* NOTE: define all ids we need here. */
    BLI_snprintf(colorids,
                 sizeof(colorids),
                 "#define TH_ACTIVE %d\n"
                 "#define TH_SELECT %d\n"
                 "#define TH_TRANSFORM %d\n"
                 "#define TH_WIRE %d\n"
                 "#define TH_CAMERA_PATH %d\n",
                 TH_ACTIVE,
                 TH_SELECT,
                 TH_TRANSFORM,
                 TH_WIRE,
                 TH_CAMERA_PATH);
    *sh = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_extra_wire_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl, datatoc_extra_wire_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def,
                                 colorids,
                                 (is_select) ? "#define SELECT_EDGES\n" : "",
                                 (use_object) ? "#define OBJECT_WIRE \n" : NULL,
                                 NULL},
    });
  }
  return *sh;
}

GPUShader *OVERLAY_shader_extra_loose_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->extra_loose_point) {
    sh_data->extra_loose_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_extra_loose_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_extra_loose_point_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->extra_loose_point;
}

GPUShader *OVERLAY_shader_extra_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->extra_point) {
    sh_data->extra_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_extra_point_vert_glsl,
                                 NULL},
        .frag =
            (const char *[]){datatoc_gpu_shader_point_varying_color_varying_outline_aa_frag_glsl,
                             NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->extra_point;
}

GPUShader *OVERLAY_shader_facing(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->facing) {
    sh_data->facing = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_facing_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl, datatoc_facing_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->facing;
}

GPUShader *OVERLAY_shader_gpencil_canvas(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->gpencil_canvas) {
    sh_data->gpencil_canvas = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_edit_gpencil_canvas_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl, datatoc_extra_frag_glsl, NULL},
    });
  }
  return sh_data->gpencil_canvas;
}

GPUShader *OVERLAY_shader_grid(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->grid) {
    sh_data->grid = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_grid_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_grid_frag_glsl,
                                 NULL},
    });
  }
  return sh_data->grid;
}

GPUShader *OVERLAY_shader_grid_background(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->grid_background) {
    sh_data->grid_background = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_edit_uv_tiled_image_borders_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_grid_background_frag_glsl, NULL},
    });
  }
  return sh_data->grid_background;
}

GPUShader *OVERLAY_shader_grid_image(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->grid_image) {
    sh_data->grid_image = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_tiled_image_borders_vert_glsl,
        NULL,
        datatoc_gpu_shader_uniform_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }
  return sh_data->grid_image;
}

GPUShader *OVERLAY_shader_edit_uv_stencil_image(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_stencil_image) {
    sh_data->edit_uv_stencil_image = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_image_vert_glsl, NULL, datatoc_image_frag_glsl, e_data.lib, NULL);
  }
  return sh_data->edit_uv_stencil_image;
}

GPUShader *OVERLAY_shader_edit_uv_mask_image(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_mask_image) {
    sh_data->edit_uv_mask_image = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_image_vert_glsl,
        NULL,
        datatoc_edit_uv_image_mask_frag_glsl,
        e_data.lib,
        NULL);
  }
  return sh_data->edit_uv_mask_image;
}

GPUShader *OVERLAY_shader_image(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->image) {
    sh_data->image = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_image_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_colormanagement_lib_glsl,
                                 datatoc_image_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->image;
}

GPUShader *OVERLAY_shader_motion_path_line(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->motion_path_line) {
    sh_data->motion_path_line = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_motion_path_line_vert_glsl,
                                 NULL},
        .geom = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_motion_path_line_geom_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_3D_smooth_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->motion_path_line;
}

GPUShader *OVERLAY_shader_motion_path_vert(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->motion_path_vert) {
    sh_data->motion_path_vert = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_motion_path_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->motion_path_vert;
}

GPUShader *OVERLAY_shader_outline_prepass(bool use_wire)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (use_wire && !sh_data->outline_prepass_wire) {
    sh_data->outline_prepass_wire = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_outline_prepass_vert_glsl,
                                 NULL},
        .geom = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_outline_prepass_geom_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_outline_prepass_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define USE_GEOM\n", NULL},
    });
  }
  else if (!sh_data->outline_prepass) {
    sh_data->outline_prepass = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_outline_prepass_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_outline_prepass_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return use_wire ? sh_data->outline_prepass_wire : sh_data->outline_prepass;
}

GPUShader *OVERLAY_shader_outline_prepass_gpencil(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->outline_prepass_gpencil) {
    sh_data->outline_prepass_gpencil = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_gpencil_common_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_outline_prepass_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_gpencil_common_lib_glsl,
                                 datatoc_outline_prepass_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def,
                                 "#define USE_GPENCIL\n",
                                 "#define UNIFORM_RESOURCE_ID\n",
                                 NULL},
    });
  }
  return sh_data->outline_prepass_gpencil;
}

GPUShader *OVERLAY_shader_outline_prepass_pointcloud(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->outline_prepass_pointcloud) {
    sh_data->outline_prepass_pointcloud = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_common_pointcloud_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_outline_prepass_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_gpencil_common_lib_glsl,
                                 datatoc_outline_prepass_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def,
                                 "#define POINTCLOUD\n",
                                 "#define INSTANCED_ATTR\n",
                                 "#define UNIFORM_RESOURCE_ID\n",
                                 NULL},
    });
  }
  return sh_data->outline_prepass_pointcloud;
}

GPUShader *OVERLAY_shader_outline_detect(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->outline_detect) {
    sh_data->outline_detect = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_fullscreen_vert_glsl, NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_outline_detect_frag_glsl,
                                 NULL},
    });
  }
  return sh_data->outline_detect;
}

GPUShader *OVERLAY_shader_paint_face(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_face) {
    sh_data->paint_face = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_face_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_uniform_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->paint_face;
}

GPUShader *OVERLAY_shader_paint_point(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_point) {
    sh_data->paint_point = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_point_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_point_varying_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->paint_point;
}

GPUShader *OVERLAY_shader_paint_texture(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_texture) {
    sh_data->paint_texture = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_texture_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_colormanagement_lib_glsl,
                                 datatoc_paint_texture_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->paint_texture;
}

GPUShader *OVERLAY_shader_paint_vertcol(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_vertcol) {
    sh_data->paint_vertcol = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_vertcol_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_paint_vertcol_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->paint_vertcol;
}

GPUShader *OVERLAY_shader_paint_weight(const bool shading)
{
  int index = shading ? 1 : 0;
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_weight[index]) {
    sh_data->paint_weight[index] = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_weight_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_globals_lib_glsl,
                                 datatoc_paint_weight_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def, shading ? "#define FAKE_SHADING\n" : "", NULL},
    });
  }
  return sh_data->paint_weight[index];
}

GPUShader *OVERLAY_shader_paint_wire(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->paint_wire) {
    sh_data->paint_wire = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_paint_wire_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_flat_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->paint_wire;
}

GPUShader *OVERLAY_shader_particle_dot(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->particle_dot) {
    sh_data->particle_dot = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_particle_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl, datatoc_particle_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define USE_DOTS\n", NULL},
    });
  }
  return sh_data->particle_dot;
}

GPUShader *OVERLAY_shader_particle_shape(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->particle_shape) {
    sh_data->particle_shape = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_particle_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_flat_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define INSTANCED_ATTR\n", NULL},
    });
  }
  return sh_data->particle_shape;
}

GPUShader *OVERLAY_shader_sculpt_mask(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->sculpt_mask) {
    sh_data->sculpt_mask = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_sculpt_mask_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_sculpt_mask_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->sculpt_mask;
}

struct GPUShader *OVERLAY_shader_uniform_color(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->uniform_color) {
    sh_data->uniform_color = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_depth_only_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_gpu_shader_uniform_color_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, NULL},
    });
  }
  return sh_data->uniform_color;
}

struct GPUShader *OVERLAY_shader_volume_velocity(bool use_needle, bool use_mac)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (use_needle && !sh_data->volume_velocity_needle_sh) {
    sh_data->volume_velocity_needle_sh = DRW_shader_create_with_lib(
        datatoc_volume_velocity_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n"
        "#define USE_NEEDLE\n");
  }
  else if (use_mac && !sh_data->volume_velocity_mac_sh) {
    sh_data->volume_velocity_mac_sh = DRW_shader_create_with_lib(
        datatoc_volume_velocity_vert_glsl,
        NULL,
        datatoc_gpu_shader_3D_smooth_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n"
        "#define USE_MAC\n");
  }
  else if (!sh_data->volume_velocity_sh) {
    sh_data->volume_velocity_sh = DRW_shader_create_with_lib(
        datatoc_volume_velocity_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }
  if (use_needle) {
    return sh_data->volume_velocity_needle_sh;
  }
  if (use_mac) {
    return sh_data->volume_velocity_mac_sh;
  }

  return sh_data->volume_velocity_sh;
}

struct GPUShader *OVERLAY_shader_volume_gridlines(bool color_with_flags, bool color_range)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->volume_gridlines_flags_sh && color_with_flags) {
    sh_data->volume_gridlines_flags_sh = DRW_shader_create_with_lib(
        datatoc_volume_gridlines_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n"
        "#define SHOW_FLAGS\n");
  }
  else if (!sh_data->volume_gridlines_range_sh && color_range) {
    sh_data->volume_gridlines_range_sh = DRW_shader_create_with_lib(
        datatoc_volume_gridlines_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n"
        "#define SHOW_RANGE\n");
  }
  else if (!sh_data->volume_gridlines_sh) {
    sh_data->volume_gridlines_sh = DRW_shader_create_with_lib(
        datatoc_volume_gridlines_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        datatoc_common_view_lib_glsl,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }

  if (color_with_flags) {
    return sh_data->volume_gridlines_flags_sh;
  }
  if (color_range) {
    return sh_data->volume_gridlines_range_sh;
  }

  return sh_data->volume_gridlines_sh;
}

GPUShader *OVERLAY_shader_wireframe_select(void)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->wireframe_select) {
    sh_data->wireframe_select = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_wireframe_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_wireframe_frag_glsl, NULL},
        .defs = (const char *[]){sh_cfg->def, "#define SELECT_EDGES\n", NULL},
    });
  }
  return sh_data->wireframe_select;
}

GPUShader *OVERLAY_shader_wireframe(bool custom_bias)
{
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const GPUShaderConfigData *sh_cfg = &GPU_shader_cfg_data[draw_ctx->sh_cfg];
  OVERLAY_Shaders *sh_data = &e_data.sh_data[draw_ctx->sh_cfg];
  if (!sh_data->wireframe[custom_bias]) {
    sh_data->wireframe[custom_bias] = GPU_shader_create_from_arrays({
        .vert = (const char *[]){sh_cfg->lib,
                                 datatoc_common_view_lib_glsl,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_gpu_shader_common_obinfos_lib_glsl,
                                 datatoc_wireframe_vert_glsl,
                                 NULL},
        .frag = (const char *[]){datatoc_common_view_lib_glsl,
                                 datatoc_common_globals_lib_glsl,
                                 datatoc_wireframe_frag_glsl,
                                 NULL},
        .defs = (const char *[]){sh_cfg->def,
                                 custom_bias ? "#define CUSTOM_DEPTH_BIAS\n" : NULL,
                                 NULL},
    });
  }
  return sh_data->wireframe[custom_bias];
}

GPUShader *OVERLAY_shader_xray_fade(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->xray_fade) {
    sh_data->xray_fade = GPU_shader_create_from_arrays({
        .vert = (const char *[]){datatoc_common_fullscreen_vert_glsl, NULL},
        .frag = (const char *[]){datatoc_xray_fade_frag_glsl, NULL},
    });
  }
  return sh_data->xray_fade;
}

/* -------------------------------------------------------------------- */
/** \name Edit UV shaders
 * \{ */

GPUShader *OVERLAY_shader_edit_uv_edges_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_edges) {
    sh_data->edit_uv_edges = DRW_shader_create_with_shaderlib(datatoc_edit_uv_edges_vert_glsl,
                                                              datatoc_edit_uv_edges_geom_glsl,
                                                              datatoc_edit_uv_edges_frag_glsl,
                                                              e_data.lib,
                                                              NULL);
  }
  return sh_data->edit_uv_edges;
}

GPUShader *OVERLAY_shader_edit_uv_edges_for_edge_select_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_edges_for_edge_select) {
    sh_data->edit_uv_edges_for_edge_select = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_edges_vert_glsl,
        datatoc_edit_uv_edges_geom_glsl,
        datatoc_edit_uv_edges_frag_glsl,
        e_data.lib,
        "#define USE_EDGE_SELECT\n");
  }
  return sh_data->edit_uv_edges_for_edge_select;
}

GPUShader *OVERLAY_shader_edit_uv_face_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_faces) {
    sh_data->edit_uv_faces = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_faces_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }
  return sh_data->edit_uv_faces;
}

GPUShader *OVERLAY_shader_edit_uv_face_dots_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_face_dots) {
    sh_data->edit_uv_face_dots = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_face_dots_vert_glsl,
        NULL,
        datatoc_gpu_shader_flat_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }
  return sh_data->edit_uv_face_dots;
}

GPUShader *OVERLAY_shader_edit_uv_verts_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_verts) {
    sh_data->edit_uv_verts = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_verts_vert_glsl, NULL, datatoc_edit_uv_verts_frag_glsl, e_data.lib, NULL);
  }

  return sh_data->edit_uv_verts;
}

GPUShader *OVERLAY_shader_edit_uv_stretching_area_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_stretching_area) {
    sh_data->edit_uv_stretching_area = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_stretching_vert_glsl,
        NULL,
        datatoc_gpu_shader_2D_smooth_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }

  return sh_data->edit_uv_stretching_area;
}

GPUShader *OVERLAY_shader_edit_uv_stretching_angle_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_stretching_angle) {
    sh_data->edit_uv_stretching_angle = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_stretching_vert_glsl,
        NULL,
        datatoc_gpu_shader_2D_smooth_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n#define STRETCH_ANGLE\n");
  }

  return sh_data->edit_uv_stretching_angle;
}

GPUShader *OVERLAY_shader_edit_uv_tiled_image_borders_get(void)
{
  OVERLAY_Shaders *sh_data = &e_data.sh_data[0];
  if (!sh_data->edit_uv_tiled_image_borders) {
    sh_data->edit_uv_tiled_image_borders = DRW_shader_create_with_shaderlib(
        datatoc_edit_uv_tiled_image_borders_vert_glsl,
        NULL,
        datatoc_gpu_shader_uniform_color_frag_glsl,
        e_data.lib,
        "#define blender_srgb_to_framebuffer_space(a) a\n");
  }
  return sh_data->edit_uv_tiled_image_borders;
}

/** \} */

static OVERLAY_InstanceFormats g_formats = {NULL};

OVERLAY_InstanceFormats *OVERLAY_shader_instance_formats_get(void)
{
  DRW_shgroup_instance_format(g_formats.pos,
                              {
                                  {"pos", DRW_ATTR_FLOAT, 3},
                              });
  DRW_shgroup_instance_format(g_formats.pos_color,
                              {
                                  {"pos", DRW_ATTR_FLOAT, 3},
                                  {"color", DRW_ATTR_FLOAT, 4},
                              });
  DRW_shgroup_instance_format(g_formats.instance_pos,
                              {
                                  {"inst_pos", DRW_ATTR_FLOAT, 3},
                              });
  DRW_shgroup_instance_format(g_formats.instance_extra,
                              {
                                  {"color", DRW_ATTR_FLOAT, 4},
                                  {"inst_obmat", DRW_ATTR_FLOAT, 16},
                              });
  DRW_shgroup_instance_format(g_formats.wire_extra,
                              {
                                  {"pos", DRW_ATTR_FLOAT, 3},
                                  {"colorid", DRW_ATTR_INT, 1},
                              });
  DRW_shgroup_instance_format(g_formats.point_extra,
                              {
                                  {"pos", DRW_ATTR_FLOAT, 3},
                                  {"colorid", DRW_ATTR_INT, 1},
                              });
  DRW_shgroup_instance_format(g_formats.instance_bone,
                              {
                                  {"inst_obmat", DRW_ATTR_FLOAT, 16},
                              });
  DRW_shgroup_instance_format(g_formats.instance_bone_stick,
                              {
                                  {"boneStart", DRW_ATTR_FLOAT, 3},
                                  {"boneEnd", DRW_ATTR_FLOAT, 3},
                                  {"wireColor", DRW_ATTR_FLOAT, 4}, /* TODO: uchar color. */
                                  {"boneColor", DRW_ATTR_FLOAT, 4},
                                  {"headColor", DRW_ATTR_FLOAT, 4},
                                  {"tailColor", DRW_ATTR_FLOAT, 4},
                              });
  DRW_shgroup_instance_format(g_formats.instance_bone_envelope_outline,
                              {
                                  {"headSphere", DRW_ATTR_FLOAT, 4},
                                  {"tailSphere", DRW_ATTR_FLOAT, 4},
                                  {"outlineColorSize", DRW_ATTR_FLOAT, 4},
                                  {"xAxis", DRW_ATTR_FLOAT, 3},
                              });
  DRW_shgroup_instance_format(g_formats.instance_bone_envelope_distance,
                              {
                                  {"headSphere", DRW_ATTR_FLOAT, 4},
                                  {"tailSphere", DRW_ATTR_FLOAT, 4},
                                  {"xAxis", DRW_ATTR_FLOAT, 3},
                              });
  DRW_shgroup_instance_format(g_formats.instance_bone_envelope,
                              {
                                  {"headSphere", DRW_ATTR_FLOAT, 4},
                                  {"tailSphere", DRW_ATTR_FLOAT, 4},
                                  {"boneColor", DRW_ATTR_FLOAT, 3},
                                  {"stateColor", DRW_ATTR_FLOAT, 3},
                                  {"xAxis", DRW_ATTR_FLOAT, 3},
                              });

  return &g_formats;
}

void OVERLAY_shader_free(void)
{
  DRW_SHADER_LIB_FREE_SAFE(e_data.lib);

  for (int sh_data_index = 0; sh_data_index < ARRAY_SIZE(e_data.sh_data); sh_data_index++) {
    OVERLAY_Shaders *sh_data = &e_data.sh_data[sh_data_index];
    GPUShader **sh_data_as_array = (GPUShader **)sh_data;
    for (int i = 0; i < (sizeof(OVERLAY_Shaders) / sizeof(GPUShader *)); i++) {
      DRW_SHADER_FREE_SAFE(sh_data_as_array[i]);
    }
  }
  struct GPUVertFormat **format = (struct GPUVertFormat **)&g_formats;
  for (int i = 0; i < sizeof(g_formats) / sizeof(void *); i++, format++) {
    MEM_SAFE_FREE(*format);
  }
}
