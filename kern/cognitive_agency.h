/*
 * Mach Operating System
 * Copyright (c) 2025 CognuMach Project
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 */

/**
 * @file cognitive_agency.h
 * @brief OpenCog-inspired Cognitive Agency for GNU Mach
 * 
 * This module implements a cognitive agency layer that brings AGI-inspired
 * capabilities to the GNU Mach microkernel. It provides knowledge representation,
 * reasoning, learning, and goal-oriented behavior for kernel-level agents.
 * 
 * Architecture:
 * - Atomspace: Distributed knowledge representation inspired by OpenCog Hyperon
 * - Agents: Autonomous entities with goals, beliefs, and actions
 * - Cognitive IPC: Enhanced message passing with semantic understanding
 * - Learning: Adaptive behavior based on system state and history
 */

#ifndef _KERN_COGNITIVE_AGENCY_H_
#define _KERN_COGNITIVE_AGENCY_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/kern_types.h>
#include <ipc/ipc_types.h>

/*
 * Cognitive Atom Types
 * Inspired by OpenCog Hyperon's atom system for knowledge representation
 */
typedef enum {
	ATOM_TYPE_CONCEPT,      /* Abstract concept representation */
	ATOM_TYPE_PREDICATE,    /* Relational predicate */
	ATOM_TYPE_LINK,         /* Connection between atoms */
	ATOM_TYPE_VALUE,        /* Concrete value */
	ATOM_TYPE_GOAL,         /* Agent goal/objective */
	ATOM_TYPE_BELIEF,       /* Agent belief state */
	ATOM_TYPE_ACTION,       /* Executable action */
	ATOM_TYPE_SCHEMA        /* Behavioral schema */
} cognitive_atom_type_t;

/*
 * Truth Value Structure
 * Represents probabilistic truth with strength and confidence
 */
typedef struct cognitive_truth_value {
	float strength;         /* Truth strength [0.0, 1.0] */
	float confidence;       /* Confidence level [0.0, 1.0] */
	unsigned int count;     /* Observation count */
} cognitive_truth_value_t;

/*
 * Cognitive Atom Link
 * Represents relationships between atoms
 */
typedef struct cognitive_atom_link {
	queue_chain_t link;                    /* Queue linkage */
	struct cognitive_atom *target;         /* Target atom */
	unsigned int link_type;                /* Type of relationship */
	float strength;                        /* Link strength [0.0, 1.0] */
} *cognitive_atom_link_t;

/*
 * Cognitive Atom
 * Basic unit of knowledge in the atomspace
 */
typedef struct cognitive_atom {
	queue_chain_t link;                    /* Queue linkage */
	cognitive_atom_type_t type;            /* Atom type */
	unsigned int id;                       /* Unique identifier */
	char name[64];                         /* Atom name */
	cognitive_truth_value_t truth;         /* Truth value */
	void *data;                            /* Type-specific data */
	unsigned int ref_count;                /* Reference counter */
	queue_head_t outgoing_links;           /* Outgoing relationships */
	queue_head_t incoming_links;           /* Incoming relationships */
	decl_simple_lock_data(, lock)         /* Synchronization */
} *cognitive_atom_t;

#define COGNITIVE_ATOM_NULL ((cognitive_atom_t) 0)

/*
 * Cognitive Agent State
 */
typedef enum {
	AGENT_STATE_IDLE,           /* Waiting for activation */
	AGENT_STATE_REASONING,      /* Performing reasoning */
	AGENT_STATE_ACTING,         /* Executing actions */
	AGENT_STATE_LEARNING,       /* Learning from experience */
	AGENT_STATE_COMMUNICATING,  /* IPC communication */
	AGENT_STATE_BLOCKED         /* Blocked on resource */
} cognitive_agent_state_t;

/*
 * Cognitive Message
 * Message in agent's message queue
 */
typedef struct cognitive_message {
	queue_chain_t link;                    /* Queue linkage */
	struct cognitive_agent *sender;        /* Sender agent */
	struct cognitive_atom *content;        /* Message content */
	unsigned int priority;                 /* Message priority */
	unsigned int timestamp;                /* Timestamp */
} *cognitive_message_t;

/*
 * Cognitive Agent
 * Autonomous entity with goals and reasoning capabilities
 */
typedef struct cognitive_agent {
	queue_chain_t link;                    /* Queue linkage */
	unsigned int id;                       /* Agent identifier */
	char name[64];                         /* Agent name */
	cognitive_agent_state_t state;         /* Current state */
	
	/* Agent cognition */
	queue_head_t goals;                    /* Goal queue */
	queue_head_t beliefs;                  /* Belief set */
	queue_head_t knowledge;                /* Knowledge base (atoms) */
	queue_head_t message_queue;            /* Incoming messages */
	unsigned int message_count;            /* Message queue size */
	
	/* IPC integration */
	ipc_port_t control_port;               /* Control port */
	ipc_port_t message_port;               /* Message port */
	
	/* Execution context */
	task_t task;                           /* Associated task */
	thread_t thread;                       /* Associated thread */
	
	/* Statistics */
	unsigned int reasoning_cycles;         /* Reasoning iterations */
	unsigned int actions_executed;         /* Actions taken */
	unsigned int messages_processed;       /* Messages handled */
	unsigned int messages_sent;            /* Messages sent */
	
	decl_simple_lock_data(, lock)         /* Synchronization */
} *cognitive_agent_t;

#define COGNITIVE_AGENT_NULL ((cognitive_agent_t) 0)

/*
 * Cognitive Atomspace
 * Global knowledge representation space
 */
typedef struct cognitive_atomspace {
	queue_head_t atoms;                    /* All atoms */
	unsigned int atom_count;               /* Total atoms */
	unsigned int max_atoms;                /* Capacity limit */
	decl_simple_lock_data(, lock)         /* Synchronization */
} *cognitive_atomspace_t;

/*
 * Cognitive Inference Rule
 * Simple rule for forward chaining inference
 */
typedef struct cognitive_rule {
	queue_chain_t link;                    /* Queue linkage */
	char name[64];                         /* Rule name */
	cognitive_atom_type_t condition_type;  /* Condition type */
	cognitive_atom_type_t conclusion_type; /* Conclusion type */
	float confidence_threshold;            /* Minimum confidence */
	unsigned int times_applied;            /* Application count */
} *cognitive_rule_t;

/*
 * Cognitive Agency System
 * Global cognitive system state
 */
typedef struct cognitive_agency {
	queue_head_t agents;                   /* Active agents */
	unsigned int agent_count;              /* Total agents */
	cognitive_atomspace_t atomspace;       /* Global atomspace */
	queue_head_t rules;                    /* Inference rules */
	unsigned int rule_count;               /* Total rules */
	boolean_t initialized;                 /* Initialization flag */
	decl_simple_lock_data(, lock)         /* Global lock */
} cognitive_agency_t;

/*
 * Global cognitive agency instance
 */
extern cognitive_agency_t global_cognitive_agency;

/*
 * Initialization and shutdown
 */
extern void cognitive_agency_init(void);
extern void cognitive_agency_shutdown(void);

/*
 * Atomspace operations
 */
extern cognitive_atomspace_t cognitive_atomspace_create(void);
extern void cognitive_atomspace_destroy(cognitive_atomspace_t space);
extern cognitive_atom_t cognitive_atom_create(
	cognitive_atomspace_t space,
	cognitive_atom_type_t type,
	const char *name);
extern void cognitive_atom_destroy(cognitive_atom_t atom);
extern cognitive_atom_t cognitive_atom_lookup(
	cognitive_atomspace_t space,
	const char *name);
extern kern_return_t cognitive_atom_set_truth(
	cognitive_atom_t atom,
	float strength,
	float confidence);
extern kern_return_t cognitive_atom_create_link(
	cognitive_atom_t from,
	cognitive_atom_t to,
	unsigned int link_type,
	float strength);
extern kern_return_t cognitive_atom_remove_link(
	cognitive_atom_t from,
	cognitive_atom_t to);
extern unsigned int cognitive_atom_count_links(
	cognitive_atom_t atom);

/*
 * Agent operations
 */
extern cognitive_agent_t cognitive_agent_create(
	const char *name,
	task_t task);
extern void cognitive_agent_destroy(cognitive_agent_t agent);
extern kern_return_t cognitive_agent_add_goal(
	cognitive_agent_t agent,
	cognitive_atom_t goal);
extern kern_return_t cognitive_agent_add_belief(
	cognitive_agent_t agent,
	cognitive_atom_t belief);
extern kern_return_t cognitive_agent_reason(
	cognitive_agent_t agent);
extern kern_return_t cognitive_agent_act(
	cognitive_agent_t agent);

/*
 * Cognitive IPC operations
 */
extern kern_return_t cognitive_agent_send_message(
	cognitive_agent_t from,
	cognitive_agent_t to,
	cognitive_atom_t message);
extern kern_return_t cognitive_agent_receive_message(
	cognitive_agent_t agent,
	cognitive_atom_t *message);
extern unsigned int cognitive_agent_pending_messages(
	cognitive_agent_t agent);
extern kern_return_t cognitive_agent_learn(
	cognitive_agent_t agent,
	cognitive_atom_t experience);

/*
 * Pattern matching and queries
 */
extern cognitive_atom_t cognitive_atomspace_find_by_type(
	cognitive_atomspace_t space,
	cognitive_atom_type_t type);
extern unsigned int cognitive_atomspace_query(
	cognitive_atomspace_t space,
	cognitive_atom_type_t type,
	cognitive_atom_t *results,
	unsigned int max_results);
extern kern_return_t cognitive_atom_traverse_links(
	cognitive_atom_t atom,
	void (*callback)(cognitive_atom_t, void *),
	void *context);

/*
 * Inference and reasoning
 */
extern cognitive_rule_t cognitive_rule_create(
	const char *name,
	cognitive_atom_type_t condition_type,
	cognitive_atom_type_t conclusion_type,
	float confidence_threshold);
extern void cognitive_rule_destroy(cognitive_rule_t rule);
extern kern_return_t cognitive_agency_add_rule(cognitive_rule_t rule);
extern kern_return_t cognitive_agent_apply_rules(
	cognitive_agent_t agent);

/*
 * Query and introspection
 */
extern unsigned int cognitive_agency_get_agent_count(void);
extern unsigned int cognitive_agency_get_atom_count(void);
extern kern_return_t cognitive_agent_get_state(
	cognitive_agent_t agent,
	cognitive_agent_state_t *state);

#endif /* _KERN_COGNITIVE_AGENCY_H_ */
