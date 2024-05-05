/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/05/05 14:08:36 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "game.h"
#include "FABRIK.h"
#include "entity/entity.h"

///////////////////////////////////////////////////////

typedef struct t_squid
{
	t_v3f		pos;
	t_v3f		rot;
	float		scale;
	t_ftree		tentacles[4];
}	t_squid;

///////////////////////////////////////////////////////

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
	__control(eng, game, dt);


	entities_update(game, dt);

	camera_update(&game->cam);
	
	ft_eng_sel_spr(eng, game->cam.surface);
	ray_render(eng, &game->map, &game->cam);
	entities_display(game);
	
	ft_eng_sel_spr(eng, NULL);
	ft_put_sprite_s(eng, game->cam.surface, (t_v2i){0, 0}, 3);

	printf("FPS: %f\n", 1.0 / dt);

	game->time += dt;

	return (1);
}

///////////////////////////////////////////////////////

int	main(int argc, char **argv)
{
	t_engine	*eng;
	t_data		data;

	if (argc != 2)
		return (ft_putstr_fd("Usage: ./Dunge <map>\n", 1), 1);
	srand(time(NULL));
	eng = ft_eng_create(250 * 4, 230 * 3, "Dunge");
	if (eng)
	{
		if (!game_init(eng, &data, argv))
		{
			ft_eng_play(eng, &data, __loop);
			game_destroy(&data);
		}
		else
			ft_putstr_fd("Error: Failed to initialise the game.\n", 1);
		ft_eng_destroy(eng);
	}
	else
		ft_putstr_fd("Error: Failed to initialise the engine.\n", 1);
	return (0);
}
