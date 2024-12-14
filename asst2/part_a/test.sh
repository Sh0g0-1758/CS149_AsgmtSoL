#!/bin/bash

# List of tasks
TASKS=(
    "simple_test_sync"
    "ping_pong_equal"
    "ping_pong_unequal"
    "super_light"
    "super_super_light"
    "recursive_fibonacci"
    "math_operations_in_tight_for_loop"
    "math_operations_in_tight_for_loop_fewer_tasks"
    "math_operations_in_tight_for_loop_fan_in"
    "math_operations_in_tight_for_loop_reduction_tree"
    "spin_between_run_calls"
    "mandelbrot_chunked"
    "simple_run_deps_test"
)

# Iterate through the list and run each task
for TASK in "${TASKS[@]}"
do
    echo "./runtasks $TASK"
    ./runtasks "$TASK"
done
