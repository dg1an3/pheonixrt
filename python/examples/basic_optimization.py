#!/usr/bin/env python3
"""
Basic Brimstone Optimization Example

This script demonstrates the basic usage of pybrimstone for
radiotherapy treatment plan optimization.

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
"""

import numpy as np

try:
    import pybrimstone as pb
except ImportError:
    print("Error: pybrimstone not installed or not built.")
    print("Please run: pip install -e . from the python/ directory")
    exit(1)


def main():
    """Run basic optimization example"""

    print("=" * 70)
    print("Brimstone Basic Optimization Example")
    print("=" * 70)
    print()

    # Create a treatment plan
    print("Creating treatment plan...")
    plan = pb.Plan()
    print(f"  {plan}")
    print()

    # Add multiple beams at different gantry angles
    print("Adding treatment beams...")
    beam_angles = [0, 45, 90, 135, 180, 225, 270, 315]
    for angle in beam_angles:
        beam = pb.Beam(gantry_angle=angle)
        print(f"  Added {beam}")

    print(f"\nPlan now has {plan.beam_count} beams")
    print()

    # Create optimizer
    print("Creating optimizer...")
    optimizer = pb.PlanOptimizer(plan)
    print(f"  {optimizer}")
    print()

    # Get initial state vector
    print("Initial state...")
    initial_weights = optimizer.get_state_vector()
    print(f"  State vector dimension: {len(initial_weights)}")
    print(f"  Initial weights range: [{initial_weights.min():.6f}, {initial_weights.max():.6f}]")
    print(f"  Initial weights mean: {initial_weights.mean():.6f}")
    print()

    # Run optimization
    print("Running optimization...")
    print("  (This may take a few minutes...)")
    result = optimizer.optimize(
        initial_weights=initial_weights,
        max_iterations=500,
        tolerance=1e-3
    )
    print()

    # Display results
    print("Optimization Results:")
    print("-" * 70)
    print(f"  Success: {result['success']}")
    print(f"  Iterations: {result['iterations']}")
    print(f"  Final cost: {result['final_cost']:.6f}")
    print()

    final_weights = result['final_weights']
    print("Final beamlet weights:")
    print(f"  Dimension: {len(final_weights)}")
    print(f"  Range: [{final_weights.min():.6f}, {final_weights.max():.6f}]")
    print(f"  Mean: {final_weights.mean():.6f}")
    print(f"  Std Dev: {final_weights.std():.6f}")
    print(f"  Non-zero beamlets: {np.count_nonzero(final_weights > 1e-6)}")
    print()

    # Compare initial vs final
    print("Change in weights:")
    weight_change = final_weights - initial_weights
    print(f"  Mean change: {weight_change.mean():.6f}")
    print(f"  Max increase: {weight_change.max():.6f}")
    print(f"  Max decrease: {weight_change.min():.6f}")
    print()

    # Update plan with final weights
    print("Updating plan with optimized weights...")
    optimizer.set_state_vector(final_weights)
    plan.update_histograms()
    print("  Plan updated successfully")
    print()

    print("=" * 70)
    print("Example completed!")
    print("=" * 70)


if __name__ == "__main__":
    main()
