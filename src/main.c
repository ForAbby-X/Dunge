/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/05/05 12:35:03 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine.h"
#include "model.h"
#include "FABRIK.h"

///////////////////////////////////////////////////////

typedef struct s_data
{
	t_camera	cam;

	t_mesh		sphere;
	t_mesh		cube;
	t_mesh		grid;

	t_v3f		pos;
	t_v2f		rot;

	t_ftree		ftree;
	t_v3f		target;

	t_v2f		rotation;

	float		time;
}	t_data;

///////////////////////////////////////////////////////

void ftree_render(t_engine *const eng, t_camera *const cam, t_data *const game, t_ftree *const tree)
{
	int		i;

	i = 0;
	while (i < (int)vector_size(&tree->segments))
	{
		t_fsegment	*segment = ftree_get_segment(tree, i);
		t_fsegment	*next_segment = ftree_get_segment(tree, i + 1);

		t_v2f const	rot = v3flook(segment->pos, next_segment->pos);

		t_transform	trans;

		if (i < (int)vector_size(&tree->segments) - 1)
		{

			trans.resize = (t_v3f){segment->size, 0.2f, 0.2f};
			trans.translation = segment->pos + v3frot((t_v3f){segment->size / 2.f}, rot);
			trans.rotation = rot;
			mesh_put(eng, cam, trans, &game->grid);
		}

		trans.rotation = (t_v2f){0.f, 0.f};
		trans.resize = (t_v3f){0.2f, 0.2f, 0.2f};
		trans.translation = segment->pos;
		mesh_put(eng, cam, trans, &game->sphere);

		i++;
	}
}

///////////////////////////////////////////////////////

static inline float __ray_plane(
	t_v3f const ray,
	t_v3f const ray_origin,
	t_v3f const plane_point)
{
	t_v3f const plane_to_ray = plane_point - ray_origin;

	float const denom = plane_to_ray[y] / ray[y];

	return (denom);
}

///////////////////////////////////////////////////////

static inline void __init(t_engine *const eng, t_data *game)
{
	game->sphere = mesh_load(eng, "models/sphere1.obj");
	game->cube = mesh_load(eng, "models/cube1.obj");
	game->grid = mesh_load(eng, "models/grid.obj");

	game->pos = (t_v3f){0, 0, 0};
	game->rot = (t_v2f){0, 0};

	game->cam = camera_create(eng, eng->screen->size);
	game->cam.pos = (t_v3f){0.0f, 0.0f, 0.0f};
	game->cam.rot = (t_v2f){0.0f, 0.0f};
	camera_update(&game->cam);

	game->ftree = ftree_create((t_v3f){0.f, 1.f, 0.f}, (t_v3f){0.f, -1.f, 0.f}, (t_v3f){0.f, 5.f, 0.f});
	for (int i = 0; i < 6; i++)
	{
		// float	ratio = (float)i / 40.f;
		ftree_add_segment(&game->ftree, (t_v3f){0.f, 0.f, 0.f}, 1.0f, (t_constraint){CONICAL, M_PI_2});
	}
	game->target = (t_v3f){0.f, 0.f, 0.f};

	game->rotation = (t_v2f){0.0f, -M_PI_2};

	game->time = 0.0;

	mlx_mouse_move(eng->mlx, eng->win, 500, 230 * 3 / 2);
}

static inline void __control(
	t_engine *const eng,
	t_data *const game,
	float const dt)
{
	t_v3f const	dir = (t_v3f){cosf(game->cam.rot[x]), 0.f, sinf(game->cam.rot[x])} * 0.8f;
	t_v3f		vel;

	vel = (t_v3f){0};
	if (ft_key(eng, 'w').hold)
		vel += dir;
	if (ft_key(eng, 's').hold)
		vel -= dir;
	if (ft_key(eng, 'a').hold)
		vel -= (t_v3f){-dir[z], 0.f, dir[x]};
	if (ft_key(eng, 'd').hold)
		vel += (t_v3f){-dir[z], 0.f, dir[x]};

	if (ft_key(eng, XK_space).hold)
		vel += (t_v3f){0.f, 1.f, 0.f};
	if (ft_key(eng, XK_Shift_L).hold)
		vel -= (t_v3f){0.f, 1.f, 0.f};

	game->cam.rot[x] += ((float)eng->mouse_x - 500) * (0.2f / 100.f);
	game->cam.rot[y] -= ((float)eng->mouse_y - 230 * 3 / 2) * (0.2f / 100.f);
	mlx_mouse_move(eng->mlx, eng->win, 500, 230 * 3 / 2);

	// game->cam.rot[y] = limit_rotation(game->cam.rot[y], 0.f, M_PI_2);

	game->cam.pos += vel * dt * 2.f;
}

///////////////////////////////////////////////////////

static inline int __loop(t_engine *eng, t_data *game, double dt)
{
	t_transform trans;

	trans.rotation = (t_v2f){0.0f, 0.0f};
	trans.resize = (t_v3f){1.0f, 1.0f, 1.0f};
	trans.translation = (t_v3f){0.0f, 0.0f, 0.0f};

	__control(eng, game, dt);
	// kit main libre
	// game->cam.rot = v3flook(game->cam.pos, (t_v3f){0.f, 0.f, 0.f});
	camera_update(&game->cam);

	// Get cam dir
	t_v3f cam_dir = v3frot((t_v3f){1.f, 0.f, 0.f}, game->cam.rot);
	t_v3f target = game->cam.pos + cam_dir * __ray_plane(cam_dir, game->cam.pos, (t_v3f){0.f, 0.f, 0.f});
	// target[y] = 0.01f;
	(void)__ray_plane;
	// game->target += (game->cam.pos - game->target) * 0.5f * (float)dt;
	// t_v3f	target = game->target;

	
	
	ftree_set_target(&game->ftree, target);
	ftree_solve(&game->ftree);
	

	camera_update(&game->cam);
	ft_eng_sel_spr(eng, game->cam.surface);
	ft_memset(game->cam.depth_buffer, 0xFF,
			  game->cam.surface->size[x] * game->cam.surface->size[y] * sizeof(float));
	ft_clear(eng, (t_color){0xFFFFFF});

	// Display the sphere
	// mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, {.0125f, .0125f, .0125f}, target}, &game->sphere);

	// Display the cube
	mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, (t_v3f){0.125f, 0.125f, 0.125f} / 2.f, {0.f, 0.f, 0.f}}, &game->sphere);

	ftree_render(eng, &game->cam, game, &game->ftree);

	// Display the grid
	// t_v3f const	dir_to_cam = __limit_rotation(game->cam.pos, (t_v3f){0.f, 1.f, 0.f}, M_PI_2 * 0.5f);

	// printf("dir_to_cam: %f %f %f\n", dir_to_cam[x], dir_to_cam[y], dir_to_cam[z]);

	// mesh_put(eng, &game->cam, (t_transform){
	// 	v3flook((t_v3f){0.f, 0.f, 0.f}, dir_to_cam),
	// 	{0.25f, 0.01f, 0.01f},
	// 	{0.f, 0.f, 0.f}
	// 	}, &game->cube);

	ft_eng_sel_spr(eng, NULL);
	ft_put_sprite(eng, game->cam.surface, (t_v2i){0, 0});

	printf("FPS: %f\n", 1.0 / dt);

	game->time += dt;

	return (1);
}

static inline void __destroy(t_data *game)
{
	ftree_destroy(&game->ftree);
	camera_destroy(&game->cam);
}

///////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	char *title = "Dunge";
	t_engine *eng;
	t_data data;

	(void)argc;
	(void)argv;
	srand(time(NULL));
	eng = ft_eng_create(250 * 4, 230 * 3, title);
	if (eng)
	{
		__init(eng, &data);
		ft_eng_play(eng, &data, __loop);
		__destroy(&data);
		ft_eng_destroy(eng);
	}
	else
		ft_putstr_fd("Error: Failed to initialise the engine.\n", 1);
	return (0);
}
