/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   game.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 16:17:55 by alde-fre          #+#    #+#             */
/*   Updated: 2024/05/05 14:01:05 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAME_H
# define GAME_H

# include "engine.h"

# include "parsing.h"
# include "raycaster.h"
# include "aabb.h"
# include "model.h"

int		game_init(t_engine *const eng, t_data *const game, char **argv);
void	game_destroy(t_data *const game);

struct s_data
{
	t_engine	*eng;
	t_map		map;
	t_camera	cam;
	float		sensitivity;
	t_sprite	*sprites[32];
	t_mesh		models[32];
	t_vector	entities;

	int			show_settings;
	float		time;
	
};

#endif
