/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   destroy.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/13 06:06:05 by alde-fre          #+#    #+#             */
/*   Updated: 2024/05/05 14:09:16 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "game.h"

void	game_models_destroy(t_data *const game)
{
	size_t	index;

	index = 0;
	while (game->models[index].spr)
		mesh_destroy(&game->models[index++]);
}

void	game_destroy(t_data *const game)
{
	camera_destroy(&game->cam);
	vector_destroy(&game->entities);
	if (game->cam.surface)
		camera_destroy(&game->cam);
	game_models_destroy(game);
}
