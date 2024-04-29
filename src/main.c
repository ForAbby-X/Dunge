/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   main.c											 :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: alde-fre <alde-fre@student.42.fr>		  +#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2023/06/21 10:36:00 by alde-fre		  #+#	#+#			 */
/*   Updated: 2024/04/28 18:04:13 by alde-fre		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#include "engine.h"
#include "model.h"

///////////////////////////////////////////////////////

typedef struct s_arm
{
	t_v3f pos;
	t_v2f rot;
	t_v3f size;

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

t_arm *arms_add(t_vector *const arms, t_v3f pos, t_v2f rot, t_v3f size)
{
	t_arm arm;

	arm.pos = pos;
	arm.rot = rot;
	arm.size = size;

	return (vector_addback(arms, &arm));
}

t_v3f limit_rotation(t_v3f direction, t_v3f axis, float max_angle)
{
	direction  = v3fnorm(direction, 1.f);
	axis  = v3fnorm(axis, 1.f);

	// Calculate the dot product between the direction vector and the axis
	float dot_product = v3fdot(direction, axis);

	// Calculate the magnitude of the direction vector and the axis
	float dir_mag = v3fmag(direction);
	float axis_mag = v3fmag(axis);

	// Calculate the angle between the direction vector and the axis (in radians)
	float angle = acosf(dot_product / (dir_mag * axis_mag));

	// Clamp the angle to the maximum allowed rotation
	float clamped_angle = fminf(angle, max_angle);

	// Calculate the new direction vector after limiting rotation
	direction[x] = axis[x] * cos(clamped_angle) + (direction[x] - axis[x] * dot_product) * sin(clamped_angle);
	direction[y] = axis[y] * cos(clamped_angle) + (direction[y] - axis[y] * dot_product) * sin(clamped_angle);
	direction[z] = axis[z] * cos(clamped_angle) + (direction[z] - axis[z] * dot_product) * sin(clamped_angle);

	return (direction);
}

void arms_point_to(t_vector *const arms, t_v3f start_pos, t_v3f target_pos)
{
	t_arm *arm;
	int i;
	t_v3f temp_pos;
	t_v3f next_pos;
	float last_size;

	t_v3f last_diff;

	t_arm	*last_arm = vector_get(arms, vector_size(arms) - 1);

	int counter = 0;
	t_v3f ddiff = target_pos - (last_arm->pos + v3frot((t_v3f){last_arm->size[x] / 2.f}, v3flook(last_arm->pos, target_pos)));
	while (counter < 10 && v3fmag(ddiff) > 0.01f)
	{
		temp_pos = target_pos;
		i = vector_size(arms) - 1;
		arm = vector_get(arms, i);

		last_diff = target_pos - arm->pos;
		while (i >= 0)
		{
			arm = vector_get(arms, i);

			t_v3f diff = temp_pos - arm->pos;
			t_v3f mov = limit_rotation(diff, last_diff, M_PI_2);

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
				next_pos = target_pos;

			t_v3f diff = arm->pos - temp_pos;
			t_v3f mov = limit_rotation(diff, last_diff, M_PI_2);

			arm->rot = v3flook(arm->pos, next_pos);

			arm->pos = temp_pos + v3fnorm(mov, last_size);
			temp_pos = arm->pos;
			last_size = arm->size[x];
			last_diff = diff;
			i++;
		}
		ddiff = target_pos - (last_arm->pos + v3frot((t_v3f){last_arm->size[x] / 2.f}, v3flook(last_arm->pos, target_pos)));
		++counter;
	}
}

void arms_render(t_engine *const eng, t_camera *const cam, t_data *const game, t_vector *const arms)
{
	t_transform trans;
	t_arm *arm;
	t_length i;

	i = 0;
	while (i < vector_size(arms))
	{
		arm = vector_get(arms, i);
		trans.rotation = arm->rot;
		trans.resize = arm->size;
		trans.resize[y] = 0.01f;
		trans.resize[z] = 0.01f;
		trans.translation = arm->pos + v3frot((t_v3f){arm->size[x] / 2.f}, arm->rot);

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

	for (int i = 0; i < 100; i++)
	{
		float width = (15 - i + 1) / 15.f;
		arms_add(&game->arms, (t_v3f){0.f, 0.1f, 0.f}, (t_v2f){0.f, 0.f}, (t_v3f){0.1f, width, width} / 10.f);
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
	if (ft_key(eng, 'z').hold)
		vel += dir;
	if (ft_key(eng, 's').hold)
		vel -= dir;
	if (ft_key(eng, 'q').hold)
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
