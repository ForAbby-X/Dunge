/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   manager.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/04/30 18:01:31 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FABRIK.h"

////////////////////////////////////////////////

static inline float __angle_between_vec(t_v3f const p0, t_v3f const p1)
{
	return (acosf(v3fdot(p0, p1)));
}

static inline t_v3f __slerp(t_v3f p0, t_v3f p1, float const t)
{
	float const	angle = __angle_between_vec(v3fnorm(p0, 1.f), v3fnorm(p1, 1.f));

	float const	sin_of_angle = sinf(angle);
	return (p0 * sinf((1 - t) * angle) / sin_of_angle + p1 * sinf(t * angle) / sin_of_angle);
}

static inline t_v3f __limit_rotation(t_v3f direction, t_v3f cone_direction, float const cone_angle)
{
	// direction = v3fnorm(direction, 1.f);
	// cone_direction = v3fnorm(cone_direction, 1.f);
	float const	current_angle = __angle_between_vec(v3fnorm(direction, 1.f), v3fnorm(cone_direction, 1.f));
	float		angle_to_edge;
	t_v3f		rectified_dir;

	rectified_dir = direction;
	if (current_angle > cone_angle)
	{
		angle_to_edge = cone_angle / current_angle;
		rectified_dir = v3fnorm(cone_direction, v3fmag(direction));
		rectified_dir = __slerp(rectified_dir, cone_direction, angle_to_edge);
	}
	return (rectified_dir);
}

////////////////////////////////////////////////

f_tree	ftree_create(t_v3f const origin, t_v3f const origin_direction, t_v3f const target)
{
	t_ftree	tree;

	tree.segments = vector_create(sizeof(t_fsegment));
	tree.origin = origin;
	tree.origin_direction = origin_direction;
	tree.target = target;
	return (tree);
}
void	ftree_destroy(t_ftree *const tree)
{
	vector_destroy(&tree->segments);
}

void	ftree_add_segment(t_ftree *const tree, t_v3f const pos, t_v3f const size, t_constraint const constraint)
{
	t_fsegment	segment;

	segment.pos = pos;
	segment.size = size;
	segment.constraint = constraint;
	segment.id = vector_size(&tree->segments);
	vector_addback(&tree->segments, &segment);
}

void	ftree_remove_segment(t_ftree *const tree, t_length const id)
{
	vector_remove(&tree->segments, id);
}

void	ftree_set_target(t_ftree *const tree, t_v3f const target)
{
	tree->target = target;
}

void	ftree_solve(t_ftree *const tree)
{
	//TODO
}