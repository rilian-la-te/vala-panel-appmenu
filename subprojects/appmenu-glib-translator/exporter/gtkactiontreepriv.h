/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include "gtkactiontree.h"
#include "gtkactionsnapshot.h"

typedef struct _GtkActionTreeWork GtkActionTreeWork;
typedef struct _GtkActionTargetBucket GtkActionTargetBucket;

struct _GtkActionTargetBucket
{
  GVariant         *target;
  GtkActionBinding *first_binding;
  guint             hash;
};

struct _GtkActionBinding
{
  GtkActionRoute            *route;
  GtkActionProvider         *provider;
  GtkActionBinding          *route_prev;
  GtkActionBinding          *route_next;
  GtkActionBinding          *provider_prev;
  GtkActionBinding          *provider_next;
  GtkActionBinding          *target_prev;
  GtkActionBinding          *target_next;
  GtkActionBinding          *dirty_next;
  GtkActionTargetBucket     *target_bucket;
  GtkActionKey              *key;
  GVariant                  *target;
  char                      *primary_accel;
  GtkActionBindingState      state;
  GtkActionBindingCallback   callback;
  gpointer                   user_data;
  GDestroyNotify             destroy;
  GtkActionBinding         **owner_location;
  GtkActionInterest          interest;
  guint                      alive : 1;
  guint                      dirty : 1;
  guint                      queued : 1;
};

struct _GtkActionSubscription
{
  GtkActionRoute                 *route;
  GtkActionProvider              *provider;
  GtkActionSubscription          *route_prev;
  GtkActionSubscription          *route_next;
  GtkActionSubscription          *provider_prev;
  GtkActionSubscription          *provider_next;
  GtkActionSubscription          *state_prev;
  GtkActionSubscription          *state_next;
  GtkActionKey                   *key;
  GVariant                       *target;
  GtkActionSnapshot               snapshot;
  GtkActionSubscriptionCallback   callback;
  gpointer                        user_data;
  GDestroyNotify                  destroy;
  GtkActionSubscription         **owner_location;
  GtkActionInterest               interest;
  guint                           queued : 1;
  guint                           cancelled : 1;
};

struct _GtkActionProvider
{
  GtkActionNode           *node;
  GtkActionKey            *key;
  GtkActionSource         *source;
  GtkActionSnapshot        snapshot;
  GtkActionBinding        *first_binding;
  GtkActionBinding        *boolean_bindings;
  GtkActionBinding        *raw_state_bindings;
  GPtrArray               *target_buckets;
  GHashTable              *target_index;
  GtkActionSubscription   *first_subscription;
  GtkActionSubscription   *first_state_subscription;
  guint64                  revision;
#ifdef G_ENABLE_DEBUG
  guint64                  state_bindings_touched;
#endif
  guint                    retired : 1;
};

struct _GtkActionPropertySource
{
  GtkActionNode     *node;
  GParamSpec        *pspec;
  GtkActionProvider *providers;
  gulong             handler_id;
};

struct _GtkActionSource
{
  GtkActionNode     *node;
  GActionGroup      *group;
  char              *prefix;
  GHashTable        *providers;
  gulong             handler_ids[4];
  guint              retired : 1;
};

struct _GtkActionRoute
{
  GtkActionNode         *node;
  GtkActionKey          *key;

  GtkActionRoute        *parent;
  GtkActionRoute        *first_child;
  GtkActionRoute        *last_child;
  GtkActionRoute        *prev_sibling;
  GtkActionRoute        *next_sibling;

  gpointer               local_provider;
  gpointer               effective_provider;
  GtkActionBinding      *first_local_binding;
  GtkActionSubscription *first_subscription;
  GtkActionResolution   *first_resolution;

  guint                  n_local_interests;
  guint                  n_subtree_interests;
  guint64                revision;
};

struct _GtkActionTreeWork
{
  GtkActionTreeWork     *next;
  GtkActionTreeCallback  callback;
  gpointer               data;
  GDestroyNotify         destroy;
};

/*
 * Nodes are plain records owned by their widget or synthetic scope. They do
 * not own either pointer; the owner must remove the node before it is freed.
 * Routes, sources, providers, bindings, and subscriptions are all reclaimed
 * through the tree so callbacks can safely mutate the topology.
 */
struct _GtkActionNode
{
  GtkActionTree  *tree;
  gpointer        owner;

  GtkActionNode  *parent;
  GtkActionNode  *first_child;
  GtkActionNode  *last_child;
  GtkActionNode  *prev_sibling;
  GtkActionNode  *next_sibling;
  GtkActionNode  *tree_prev;
  GtkActionNode  *next;

  GtkActionNode  *synthetic_parent;
  GtkActionNode  *first_synthetic_child;
  GtkActionNode  *synthetic_prev;
  GtkActionNode  *synthetic_next;
  GHashTable     *routes;
  GHashTable     *sources;
  GHashTable     *primary_accels;
  guint           auto_prunable : 1;
  guint           retired : 1;
};

struct _GtkActionTree
{
  GtkActionNode     *nodes;
  GtkActionTreeWork *dirty_head;
  GtkActionTreeWork *dirty_tail;
  GtkActionBinding  *dirty_binding_head;
  GtkActionBinding  *dirty_binding_tail;
  GtkActionTreeWork *retired;
  guint              update_depth;
  guint              dispatch_depth;
  guint              committing : 1;
};