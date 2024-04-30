/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FABRIK.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alde-fre <alde-fre@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/21 10:37:59 by vmuller           #+#    #+#             */
/*   Updated: 2024/04/30 17:47:11 by alde-fre         ###   ########.fr       */
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

f_tree	ftree_create(t_v3f const origin, t_v3f const origin_direction, t_v3f const target);
void	ftree_destroy(t_ftree *const tree);

void	ftree_add_segment(t_ftree *const tree, t_v3f const pos, t_v3f const size, t_constraint const constraint);
void	ftree_remove_segment(t_ftree *const tree, t_length const id);

void	ftree_set_target(t_ftree *const tree, t_v3f const target);

void	ftree_solve(t_ftree *const tree);


enum e_constraint_type
{
	BALL	= 1,
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
	t_v3f			size;
	t_constraint	constraint;
	
	t_length		id;
};

struct s_ftree
{
	t_vector	segments;
	t_v3f		origin;
	t_v3f		origin_direction;
	t_v3f 		target;
};

#endif
