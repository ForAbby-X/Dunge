/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 05:54:24 by alde-fre          #+#    #+#             */
/*   Updated: 2024/05/06 09:25:11 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "game.h"
#include "parsing.h"
#include "entity/entity.h"
#include "entity/all.h"

static int	__sprite_init(t_engine *const eng, t_data *const game)
{
	(void)eng;
	game->sprites[0] = NULL;
	return (0);
}

static int	__models_init(t_engine *const eng, t_data *const game)
{
	game->models[0] = mesh_load(eng, "models/sphere1.obj");
	game->models[1] = mesh_load(eng, "models/cube1.obj");
	game->models[2] = mesh_load(eng, "models/grid.obj");
	game->models[3] = (t_mesh){0};
	return (0);
}

int	game_init(t_engine *const eng, t_data *const game, char **argv)
{
	*game = (t_data){.eng = eng};
	game->map = pars_file(eng, argv[1]);
	if (game->map.data == NULL)
		return (1);	
	game->cam = camera_create(eng, eng->screen->size / 3);
	game->cam.pos = game->map.spawn;
	game->cam.rot = game->map.spawn_dir;
	__sprite_init(eng, game);
	__models_init(eng, game);
	game->entities = vector_create(sizeof(t_entity));
	game->show_settings = 0;
	game->time = 0.0f;
	mlx_mouse_move(eng->mlx, eng->win, eng->screen->size[x] / 2, eng->screen->size[y] / 2);

	e_squid_add(game, game->map.spawn, (t_v2f){0.f, 0.f});

	return (0);
}
