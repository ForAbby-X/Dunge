/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   manager.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/03 08:05:41 by alde-fre          #+#    #+#             */
/*   Updated: 2024/05/05 14:12:46 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "entity/all.h"
#include "particle/particle.h"

typedef struct s_squid
{
	// t_ftree		tentacles[4];
}	t_squid;

static void	_squid_update(
				t_entity *const self,
				t_data *const game,
				float const dt)
{
	(void)self;
	(void)game;
	(void)dt;
}

static void	_squid_display(t_entity *const self, t_data *const game)
{
	t_transform	transform;
	transform.rotation = self->rot;
	transform.resize = (t_v3f){1.f, 1.f, 1.f} * sinf(self->time_alive * 2.f) * 0.1f + 1.f;
	transform.translation = self->aabb.pos;
	
	mesh_put(game->eng, &game->cam, transform, self->mesh);
}

static void	_squid_destroy(t_entity *const self, t_data *const game)
{
	(void)self;
	(void)game;
}

t_entity	*e_squid_add(t_data *const game, t_v3f const pos, t_v2f const rot)
{
	t_entity	*ent;

	ent = entity_add(game, pos);
	if (ent == NULL)
		return (NULL);
	ent->update = &_squid_update;
	ent->display = &_squid_display;
	ent->destroy = &_squid_destroy;
	ent->dir = (t_v3f){0};
	ent->rot = rot;
	ent->max_health = 300.f;
	ent->health = ent->max_health;
	ent->mesh = &game->models[0];
	ent->aabb = (t_aabb){pos - (t_v3f){0.16f, 0.f, 0.16f},
	{0.32f, 0.825f, 0.32f}, AABB_MOVABLE};
	ent->type = ENTITY_SQUID;
	return (ent);
}
