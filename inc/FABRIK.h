/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FABRIK.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/05/02 17:24:51 by alde-fre         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FABRIK_H
# define FABRIK_H

# include "vector.h"
# include "vec3.h"

typedef enum e_constraint_type	t_constraint_type;

typedef struct s_constraint		t_constraint;
typedef struct s_fsegment		t_fsegment;
typedef struct s_ftree			t_ftree;

t_ftree		ftree_create(t_v3f const origin, t_v3f const origin_direction, t_v3f const target);
void		ftree_destroy(t_ftree *const tree);

void		ftree_add_segment(t_ftree *const tree, t_v3f const pos, float const size, t_constraint const constraint);
t_fsegment	*ftree_get_segment(t_ftree *const tree, int const id);
void		ftree_remove_segment(t_ftree *const tree, t_length const id);

void		ftree_set_origin(t_ftree *const tree, t_v3f const origin);
void		ftree_set_target(t_ftree *const tree, t_v3f const target);

void		ftree_solve(t_ftree *const tree);


enum e_constraint_type
{
	CONICAL	= 1,
	HINGE	= 2,
};

struct s_constraint
{
	t_constraint_type	type;
	float				value;
};

struct s_fsegment
{
	t_v3f			pos;
	float			size;
	t_constraint	constraint;
	
	t_length		id;
};

struct s_ftree
{
	t_vector		segments;
	t_fsegment		origin;
	t_v3f			origin_direction;
	t_fsegment		target;
};

#endif
