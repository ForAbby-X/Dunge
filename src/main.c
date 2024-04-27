/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:36:00 by alde-fre          #+#    #+#             */
/*   Updated: 2024/04/27 18:01:19 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine.h"
#include "model.h"

// #include "particle/particle.h"

typedef struct s_data
{
	t_camera	cam;
	
	t_mesh		sphere;
	t_mesh		cube;

	t_v3f		pos;
	t_v2f		rot;

	t_vector 	arms;

	float		time;
}	t_data;

static inline float	__ray_plane(
	t_v3f const ray,
	t_v3f const ray_origin,
	t_v3f const plane_point)
{
	t_v3f const	plane_to_ray = plane_point - ray_origin;

	float const	denom = plane_to_ray[y] / ray[y];

	return (denom);
}

static inline void	__init(t_engine *const eng, t_data *game)
{
	game->sphere = mesh_load(eng, "models/sphere.obj");
	game->cube = mesh_load(eng, "models/cube.obj");

	game->pos = (t_v3f){0, 0, 0};
	game->rot = (t_v2f){0, 0};

	game->cam = camera_create(eng, eng->screen->size);
	game->cam.pos = (t_v3f){0.0f, 0.0f, 0.0f};
	game->cam.rot = (t_v2f){0.0f, 0.0f};
	camera_update(&game->cam);

	game->arms = vector_create(sizeof(t_v3f));

	game->time = 0.0;

	mlx_mouse_move(eng->mlx, eng->win, 500, 260);
}

static inline void	__control(
	t_engine *const eng,
	t_data *const game,
	float const dt)
{
	t_v3f const	dir = {cosf(game->cam.rot[x]), 0.f, sinf(game->cam.rot[x])};
	t_v3f		vel;

	vel = (t_v3f){0};
	if (ft_key(eng, 'w').hold)
		vel += dir;
	if (ft_key(eng, 's').hold)
		vel -= dir;
	if (ft_key(eng, 'd').hold)
		vel += (t_v3f){-dir[z], 0.f, dir[x]};
	if (ft_key(eng, 'a').hold)
		vel -= (t_v3f){-dir[z], 0.f, dir[x]};

	if (ft_key(eng, XK_space).hold)
		vel += (t_v3f){0.f, 1.f, 0.f};
	if (ft_key(eng, XK_Shift_L).hold)
		vel -= (t_v3f){0.f, 1.f, 0.f};

	game->cam.rot[x] += ((float)eng->mouse_x - 500)
		* (0.2f / 100.f);
	game->cam.rot[y] -= ((float)eng->mouse_y - 260)
		* (0.2f / 100.f);
	mlx_mouse_move(eng->mlx, eng->win, 500, 260);
	
	game->cam.pos += vel * dt * 2.f;
}


static inline int	__loop(t_engine *eng, t_data *game, double dt)
{
	t_transform	trans;

	trans.rotation = (t_v2f){0.0f, 0.0f};
	trans.resize = (t_v3f){1.0f, 1.0f, 1.0f};
	trans.translation = (t_v3f){0.0f, 0.0f, 0.0f};

	__control(eng, game, dt);
	camera_update(&game->cam);

	// game->cam.rot = v3flook(game->cam.pos, (t_v3f){0.f, 0.f, 0.f});
	
	camera_update(&game->cam);
	ft_eng_sel_spr(eng, game->cam.surface);
	ft_memset(game->cam.depth_buffer, 0xFF,
		game->cam.surface->size[x] * game->cam.surface->size[y] * sizeof(float));
	ft_clear(eng, (t_color){0xFFFFFF});
	
	// Get cam dir
	t_v3f cam_dir = v3frot((t_v3f){1.f, 0.f, 0.f}, game->cam.rot);

	t_v3f hit_pos = game->cam.pos + cam_dir * __ray_plane(cam_dir, game->cam.pos, (t_v3f){0.f, 0.f, 0.f});

	//Display the sphere
	mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, {.125f, .125f, .125f},
		hit_pos}, &game->sphere);

	//Display the cube
	mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, {0.125f, 1.f, 0.125f},
		(t_v3f){0.f, 0.f, 0.f}}, &game->cube);

	ft_eng_sel_spr(eng, NULL);
	ft_put_sprite(eng, game->cam.surface, (t_v2i){0, 0});

	printf("FPS: %f\n", 1.0 / dt);

	game->time += dt;
	
	return (1);
}

static inline void	__destroy(t_data *game)
{
	vector_destroy(&game->arms);
	camera_destroy(&game->cam);
}

int	main(int argc, char **argv)
{
	t_engine	*eng;
	t_data		data;

	(void)argc;
	(void)argv;
	srand(time(NULL));
	eng = ft_eng_create(250 * 4, 230 * 3, "Dunge");
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
