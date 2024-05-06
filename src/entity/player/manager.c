/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   manager.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/03 08:05:41 by alde-fre          #+#    #+#             */
/*   Updated: 2024/05/06 11:47:52 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "entity/all.h"
#include "particle/particle.h"
#include "FABRIK.h"

typedef struct s_squid
{
	t_ftree		tentacle;
}	t_squid;

static void	_init_tentacles(
				t_entity *const self,
				t_data *const game)
{
	t_squid *const	squid = (t_squid *)self->data;
	int				i;
	int				j;

	(void)game;
	i = 0;
	while (i < 1)
	{
		squid->tentacle = ftree_create(self->aabb.pos, (t_v3f){0.f, -1.f, 0.f}, self->aabb.pos - (t_v3f){0.f, 10.f, 0.f});
		j = 0;
		while (j < 4)
		{
			ftree_add_segment(&squid->tentacle, (t_v3f){0.f, 0.f, 0.f}, 0.8f, (t_constraint){CONICAL, M_PI_2});
			++j;
		}
		++i;
	}
}

static void	_tentacles_render(
				t_entity *const self,
				t_data *const game)
{
	t_squid *const	squid = (t_squid *)self->data;
	t_transform		transform;
	t_length		i;
	t_length		j;

	i = 0;
	while (i < 1)
	{
		j = 0;
		while (j + 1 < vector_size(&squid->tentacle.segments))
		{
			t_fsegment	*segment = ftree_get_segment(&squid->tentacle, j);
			t_fsegment	*next_segment = ftree_get_segment(&squid->tentacle, j + 1);
			t_v2f		rot = v3flook(segment->pos, next_segment->pos);
			t_v3f		dir = v3frot((t_v3f){segment->size, 0.f, 0.f}, rot);

			transform.translation = segment->pos + dir / 2.f;
			transform.rotation = rot;
			transform.resize = (t_v3f){segment->size, 0.15f, 0.15f};
			mesh_put(game->eng, &game->cam, transform, &game->models[1]);
			++j;
		}
		++i;
	}
}



static void	_squid_update(
				t_entity *const self,
				t_data *const game,
				float const dt)
{
	t_squid *const	squid = (t_squid *)self->data;
	// t_v3f const		diff = game->cam.pos - self->aabb.pos;
	(void)dt;

	ftree_set_origin(&squid->tentacle, self->aabb.pos);
	ftree_set_target(&squid->tentacle, game->cam.pos);
	ftree_solve(&squid->tentacle);
}

static void	_squid_display(t_entity *const self, t_data *const game)
{
	t_transform	transform;
	transform.rotation = self->rot;
	transform.resize = (t_v3f){1.f, 1.f, 1.f} * sinf(self->time_alive * 2.f) * 0.1f + 1.f;
	transform.translation = self->aabb.pos;
	
	mesh_put(game->eng, &game->cam, transform, self->mesh);
	_tentacles_render(self, game);

}

static void	_squid_destroy(t_entity *const self, t_data *const game)
{
	free(self->data);
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
	ent->aabb = (t_aabb){pos, {0.f, 0.f, 0.f}, AABB_MOVABLE};
	ent->type = ENTITY_SQUID;
	
	ent->data = malloc(sizeof(t_squid));
	*(t_squid *)ent->data = (t_squid){0};
	_init_tentacles(ent, game);

	return (ent);
}
