/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   manager.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/05/05 12:34:25 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FABRIK.h"

////////////////////////////////////////////////

t_ftree	ftree_create(t_v3f const origin, t_v3f const origin_direction, t_v3f const target)
{
	t_ftree	tree;

	tree.segments = vector_create(sizeof(t_fsegment));
	ftree_set_origin(&tree, origin);
	tree.origin_direction = origin_direction;
	ftree_set_target(&tree, target);
	return (tree);
}

void	ftree_destroy(t_ftree *const tree)
{
	vector_destroy(&tree->segments);
}

void	ftree_add_segment(t_ftree *const tree, t_v3f const pos, float const size, t_constraint const constraint)
{
	t_fsegment	segment;

	segment.pos = pos;
	segment.size = size;
	segment.constraint = constraint;
	segment.id = vector_size(&tree->segments);
	vector_addback(&tree->segments, &segment);
}

t_fsegment	*ftree_get_segment(t_ftree *const tree, int const id)
{
	if (id < 0)
		return (&tree->origin);
	if (id >= (int)vector_size(&tree->segments))
		return (&tree->target);
	return (vector_get(&tree->segments, id));
}

void	ftree_remove_segment(t_ftree *const tree, t_length const id)
{
	vector_erase(&tree->segments, id);
}

void	ftree_set_origin(t_ftree *const tree, t_v3f const origin)
{
	t_fsegment	segment;

	segment.pos = origin;
	segment.size = 0.0f;
	segment.constraint = (t_constraint){CONICAL, 0.0f}; // Hard constraint so the target stay straight
	segment.id = -1;

	tree->origin = segment;
}

void	ftree_set_target(t_ftree *const tree, t_v3f const target)
{
	t_fsegment	segment;

	segment.pos = target;
	segment.size = 0.0f;								// This one is ignored because it's only a point in space
	segment.constraint = (t_constraint){CONICAL, 0.0f};	// This one is ignored because it's only a point in space
	segment.id = -1;									// This one is ignored because it's only a point in space

	tree->target = segment;
}

////////////////////////////////////////////////

static inline float __angle_between_vec(t_v3f const p0, t_v3f const p1)
{
	return (acosf(v3fdot(v3fnorm(p0, 1.f), v3fnorm(p1, 1.f))));
}

static inline t_v3f __slerp(t_v3f p0, t_v3f p1, float const t)
{
	float const	angle = __angle_between_vec(p0, p1);
	float const	sin_of_angle = sinf(angle);
	
	return (p0 * sinf((1 - t) * angle) / sin_of_angle + p1 * sinf(t * angle) / sin_of_angle);
}

static inline t_v3f __limit_rotation(t_v3f direction, t_v3f cone_direction, float const cone_angle)
{
	float		current_angle = __angle_between_vec(direction, cone_direction);
	t_v3f		rectified_dir;

	rectified_dir = direction;
	if (current_angle > cone_angle)
	{
		// if (current_angle < 0.0001f)
		// 	current_angle += rand() / (float)RAND_MAX * 0.0001f;

		float	percentage_to_edge = cone_angle / current_angle;
		
		rectified_dir = __slerp(cone_direction, rectified_dir, percentage_to_edge);
		// rectified_dir = v3fnorm(cone_direction, v3f(direction));
	}
	return (rectified_dir);
}

////////////////////////////////////////////////

static inline t_v3f	__ftree_constrain_between(t_ftree *const tree, t_fsegment *const segment)
{
	// TODO: add constraint type (for now only CONICAL constraint is implemented)
	
	t_fsegment		*before_segment = ftree_get_segment(tree, segment->id - 1);
	t_fsegment		*after_segment = ftree_get_segment(tree, segment->id + 1);

	t_constraint	constraint = segment->constraint;

	t_v3f			before_to_actual = segment->pos - before_segment->pos;
	t_v3f			actual_to_after = after_segment->pos - segment->pos;

	t_v3f			rectified_diff = __limit_rotation(actual_to_after, before_to_actual, constraint.value);

	return (rectified_diff);
}

static inline t_v3f	__ftree_constrain_between_rev(t_ftree *const tree, t_fsegment *const segment)
{
	// TODO: add constraint type (for now only CONICAL constraint is implemented)

	t_fsegment		*before_segment = ftree_get_segment(tree, segment->id - 1);
	t_fsegment		*after_segment = ftree_get_segment(tree, segment->id + 1);

	t_constraint	constraint = before_segment->constraint;
	
	t_v3f			before_to_actual = segment->pos - before_segment->pos;
	t_v3f			actual_to_after = after_segment->pos - segment->pos;

	t_v3f			rectified_diff = __limit_rotation(-before_to_actual, -actual_to_after, constraint.value);

	return (rectified_diff);
}

static inline void	__ftree_solve_backward(t_ftree *const tree)
{
	int	i;

	i = (int)vector_size(&tree->segments) - 1;
	while (i >= 0)
	{
		t_fsegment	*segment = ftree_get_segment(tree, i);
		t_fsegment	*next_segment = ftree_get_segment(tree, i + 1);


		if (i == (int)vector_size(&tree->segments) - 1)
		{
			segment->pos = tree->target.pos - v3fnorm(tree->target.pos - segment->pos, 0.0001f);
		}
		else
		{
			t_v3f	corrected_dir = __ftree_constrain_between_rev(tree, next_segment);
			// (void)__ftree_constrain_between_rev;
			// t_v3f 	corrected_dir = segment->pos - next_segment->pos;

			segment->pos = next_segment->pos + v3fnorm(corrected_dir, next_segment->size);
		}
		
		--i;
	}
}

static inline void	__ftree_solve_forward(t_ftree *const tree)
{
	int	i;

	i = 0;
	while (i < (int)vector_size(&tree->segments))
	{
		t_fsegment	*prev_segment = ftree_get_segment(tree, i - 1);
		t_fsegment	*segment = ftree_get_segment(tree, i);

		if (i == 0)
		{	
			segment->pos = tree->origin.pos + v3fnorm(tree->origin_direction, 0.0001f);
		}
		else
		{
			t_v3f	corrected_dir = __ftree_constrain_between(tree, prev_segment);
			// (void)__ftree_constrain_between;
			// t_v3f 	corrected_dir = segment->pos - prev_segment->pos;

			segment->pos = prev_segment->pos + v3fnorm(corrected_dir, prev_segment->size);
		}

		++i;
	}
}

void	ftree_solve(t_ftree *const tree)
{
	t_fsegment	*last_segment = ftree_get_segment(tree, (int)vector_size(&tree->segments) - 1);
	t_v3f		diff = tree->target.pos - last_segment->pos;
	float		distance = v3fmag(diff);
	int			i;

	i = 0;
	while (i < 10 && distance > 0.01f)
	{
		__ftree_solve_backward(tree);
		__ftree_solve_forward(tree);
		++i;
	}
}