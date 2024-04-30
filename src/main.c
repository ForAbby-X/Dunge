/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/04/30 17:49:09 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine.h"
#include "model.h"
#include "FABRIK.h"

///////////////////////////////////////////////////////

typedef struct s_arm
{
	t_v3f pos;
	t_v2f rot;
	t_v3f size;
	float limit;

} t_arm;

typedef struct s_data
{
	t_camera cam;

	t_mesh sphere;
	t_mesh cube;

	t_v3f pos;
	t_v2f rot;

	t_vector arms;

	t_v3f target;

	float time;
} t_data;

///////////////////////////////////////////////////////

t_arm *arms_add(t_vector *const arms, t_v3f const pos, t_v2f const rot, t_v3f const size, float const limit)
{
	t_arm arm;

	arm.pos = pos;
	arm.rot = rot;
	arm.size = size;
	arm.limit = limit;

	return (vector_addback(arms, &arm));
}

///////////////////////////////////////////////////////

float angle_between_vec(t_v3f const p0, t_v3f const p1)
{
	return (acosf(v3fdot(p0, p1)));
}

t_v3f slerp(t_v3f p0, t_v3f p1, float const t)
{
	float const	angle = angle_between_vec(v3fnorm(p0, 1.f), v3fnorm(p1, 1.f));

	float const	sin_of_angle = sinf(angle);
	return (p0 * sinf((1 - t) * angle) / sin_of_angle + p1 * sinf(t * angle) / sin_of_angle);
}

t_v3f limit_rotation(t_v3f direction, t_v3f cone_direction, float const cone_angle)
{
	// direction = v3fnorm(direction, 1.f);
	// cone_direction = v3fnorm(cone_direction, 1.f);
	float const	current_angle = angle_between_vec(v3fnorm(direction, 1.f), v3fnorm(cone_direction, 1.f));
	float		angle_to_edge;
	t_v3f		rectified_dir;

	rectified_dir = direction;
	if (current_angle > cone_angle)
	{
		angle_to_edge = cone_angle / current_angle;
		rectified_dir = v3fnorm(cone_direction, v3fmag(direction));
		rectified_dir = slerp(rectified_dir, cone_direction, angle_to_edge);
	}
	return (rectified_dir);
}

///////////////////////////////////////////////////////

void arms_point_to(t_vector *const arms, t_v3f start_pos, t_v3f target_pos)
{
	t_arm *arm;
	int i;
	t_v3f temp_pos;
	t_v3f next_pos;
	float last_size;

	t_v3f last_diff;

	int counter = 0;

	// t_v3f	start_diff = target_pos - start_pos;
	// target_pos = start_pos + v3fnorm(start_diff, v3fmag(start_diff));

	while (counter < 1)
	{
		temp_pos = target_pos;
		i = vector_size(arms) - 1;
		arm = vector_get(arms, i);

		last_diff = target_pos - arm->pos;
		while (i >= 0)
		{
			arm = vector_get(arms, i);

			t_v3f diff = temp_pos - arm->pos;
			t_v3f mov = limit_rotation(diff, last_diff, arm->limit);

			arm->pos = temp_pos - v3fnorm(mov, arm->size[x]);
			temp_pos = arm->pos;
			last_diff = diff;
			i--;
		}

		temp_pos = start_pos;
		last_size = 0.f;
		last_diff = (t_v3f){0.f, 1.f, 0.f};
		i = 0;
		while (i < (int)vector_size(arms))
		{
			arm = vector_get(arms, i);
			if (i < (int)vector_size(arms) - 1)
				next_pos = ((t_arm *)vector_get(arms, i + 1))->pos;
			else
			{
				next_pos = target_pos;
				last_size = 0.0f;
			}

			t_v3f diff = arm->pos - temp_pos;
			t_v3f mov = limit_rotation(diff, last_diff, arm->limit);

			arm->pos = temp_pos + v3fnorm(mov, last_size);

			temp_pos = arm->pos;
			last_size = arm->size[x];
			last_diff = diff;
			i++;
		}
		++counter;
	}
}

void arms_render(t_engine *const eng, t_camera *const cam, t_data *const game, t_vector *const arms)
{
	t_transform trans;
	t_arm *arm;
	int i;


	i = 0;
	while (i < (int)vector_size(arms) - 1)
	{
		arm = vector_get(arms, i);

		t_v2f const	rot = v3flook(arm->pos, ((t_arm *)vector_get(arms, i + 1))->pos);

		trans.rotation = rot;
		trans.resize = arm->size;
		trans.translation = arm->pos + v3frot((t_v3f){arm->size[x] / 2.f}, rot);

		mesh_put(eng, cam, trans, &game->cube);
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

	game->pos = (t_v3f){0, 0, 0};
	game->rot = (t_v2f){0, 0};

	game->cam = camera_create(eng, eng->screen->size);
	game->cam.pos = (t_v3f){0.0f, 0.0f, 0.0f};
	game->cam.rot = (t_v2f){0.0f, 0.0f};
	camera_update(&game->cam);

	game->arms = vector_create(sizeof(t_arm));

	game->target = game->cam.pos;

	for (int i = 0; i < 40; i++)
	{
		float	ratio = i / 40.f;
		arms_add(&game->arms,
			(t_v3f){0.f, i, 0.f},
			(t_v2f){0.f, 0.f},
			(t_v3f){0.8f, 0.4f * (1.f - ratio), 0.4f * (1.f - ratio)} / 10.f,
			M_PI_2 * (1.f - ratio));
	}

	game->time = 0.0;

	mlx_mouse_move(eng->mlx, eng->win, 500, 230 * 3 / 2);
}

static inline void __control(
	t_engine *const eng,
	t_data *const game,
	float const dt)
{
	t_v3f const dir = {cosf(game->cam.rot[x]), 0.f, sinf(game->cam.rot[x])};
	t_v3f vel;

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

	t_v3f diff = game->cam.pos - game->target;
	if (diff[x] * diff[x] + diff[y] * diff[y] + diff[z] * diff[z] > 0.25f * 0.25f)
		game->target += v3fnorm(diff, 1.f) * (float)dt * 0.5f;

	// Get cam dir
	t_v3f cam_dir = v3frot((t_v3f){1.f, 0.f, 0.f}, game->cam.rot);
	t_v3f target = game->cam.pos + cam_dir * __ray_plane(cam_dir, game->cam.pos, (t_v3f){0.f, 0.f, 0.f});
	// target[y] = 0.01f;
	// target = game->target;

	arms_point_to(&game->arms, (t_v3f){0.f, 0.0f, 0.f}, target);

	camera_update(&game->cam);
	ft_eng_sel_spr(eng, game->cam.surface);
	ft_memset(game->cam.depth_buffer, 0xFF,
			  game->cam.surface->size[x] * game->cam.surface->size[y] * sizeof(float));
	ft_clear(eng, (t_color){0xFFFFFF});

	// Display the sphere
	mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, {.0125f, .0125f, .0125f}, target}, &game->sphere);

	// Display the cube
	mesh_put(eng, &game->cam, (t_transform){{0.f, 0.f}, {0.125f, 0.125f, 0.125f}, (t_v3f){0.f, 0.f, 0.f}}, &game->sphere);

	arms_render(eng, &game->cam, game, &game->arms);

	ft_eng_sel_spr(eng, NULL);
	ft_put_sprite(eng, game->cam.surface, (t_v2i){0, 0});

	printf("FPS: %f\n", 1.0 / dt);

	game->time += dt;

	return (1);
}

static inline void __destroy(t_data *game)
{
	vector_destroy(&game->arms);
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
