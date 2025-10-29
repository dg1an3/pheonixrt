#!/usr/bin/env python3
"""
Live Visualization Example (Phase 3/4 Feature)

This script demonstrates real-time monitoring of Brimstone optimization
with visualization callbacks. This feature will be implemented in Phase 3.

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
"""

import numpy as np
import matplotlib.pyplot as plt
from typing import Dict, List

try:
    import pybrimstone as pb
except ImportError:
    print("Error: pybrimstone not installed or not built.")
    print("Please run: pip install -e . from the python/ directory")
    exit(1)


class LiveVisualizationCallback:
    """
    Callback for live optimization monitoring

    NOTE: This is a placeholder for Phase 3 implementation.
    The callback mechanism needs to be implemented in the Cython wrapper.
    """

    def __init__(self, update_interval: int = 10):
        self.update_interval = update_interval
        self.history = {
            'iteration': [],
            'cost': [],
            'gradient_norm': [],
        }
        self.fig = None
        self.axes = None

    def setup_plots(self):
        """Initialize matplotlib figures"""
        self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 10))
        self.fig.suptitle('Brimstone Optimization - Live Monitoring')
        plt.ion()  # Interactive mode
        plt.show()

    def __call__(
        self,
        iteration: int,
        level: int,
        cost: float,
        gradient_norm: float,
        beamlet_weights: np.ndarray,
    ) -> bool:
        """
        Callback function called at each iteration

        Args:
            iteration: Current iteration number
            level: Current pyramid level
            cost: Current objective function value
            gradient_norm: L2 norm of gradient
            beamlet_weights: Current beamlet weights

        Returns:
            True to continue optimization, False to terminate
        """
        # Store history
        self.history['iteration'].append(iteration)
        self.history['cost'].append(cost)
        self.history['gradient_norm'].append(gradient_norm)

        # Update plots every N iterations
        if iteration % self.update_interval == 0:
            self.update_plots(level, beamlet_weights)

        # Return True to continue (could add early stopping logic here)
        return True

    def update_plots(self, level: int, weights: np.ndarray):
        """Update visualization plots"""
        if self.fig is None:
            return

        # Clear axes
        for ax in self.axes.flat:
            ax.clear()

        # Plot 1: Convergence curve
        ax = self.axes[0, 0]
        ax.plot(self.history['iteration'], self.history['cost'], 'b-', linewidth=2)
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Cost')
        ax.set_title(f'Convergence (Level {level})')
        ax.grid(True, alpha=0.3)

        # Plot 2: Gradient norm
        ax = self.axes[0, 1]
        ax.semilogy(
            self.history['iteration'], self.history['gradient_norm'], 'r-', linewidth=2
        )
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Gradient Norm (log scale)')
        ax.set_title('Gradient Magnitude')
        ax.grid(True, alpha=0.3)

        # Plot 3: Beamlet weight histogram
        ax = self.axes[1, 0]
        ax.hist(weights[weights > 1e-6], bins=50, alpha=0.7, color='green')
        ax.set_xlabel('Beamlet Weight')
        ax.set_ylabel('Count')
        ax.set_title(f'Weight Distribution (n={np.sum(weights > 1e-6)})')
        ax.grid(True, alpha=0.3)

        # Plot 4: Summary statistics
        ax = self.axes[1, 1]
        ax.axis('off')
        stats_text = f"""
        Iteration: {self.history['iteration'][-1]}
        Pyramid Level: {level}

        Current Cost: {self.history['cost'][-1]:.6f}
        Gradient Norm: {self.history['gradient_norm'][-1]:.6e}

        Beamlet Statistics:
        - Total beamlets: {len(weights)}
        - Non-zero: {np.sum(weights > 1e-6)}
        - Mean weight: {weights.mean():.6f}
        - Max weight: {weights.max():.6f}
        """
        ax.text(0.1, 0.5, stats_text, fontsize=10, family='monospace', verticalalignment='center')
        ax.set_title('Statistics')

        plt.tight_layout()
        plt.draw()
        plt.pause(0.01)

    def plot_final_results(self):
        """Plot final optimization results"""
        if len(self.history['iteration']) == 0:
            print("No optimization history to plot")
            return

        plt.ioff()  # Turn off interactive mode

        fig, axes = plt.subplots(2, 1, figsize=(10, 8))

        # Convergence plot
        ax = axes[0]
        ax.plot(self.history['iteration'], self.history['cost'], 'b-', linewidth=2, label='Cost')
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Cost')
        ax.set_title('Final Convergence History')
        ax.legend()
        ax.grid(True, alpha=0.3)

        # Gradient norm plot
        ax = axes[1]
        ax.semilogy(
            self.history['iteration'],
            self.history['gradient_norm'],
            'r-',
            linewidth=2,
            label='Gradient Norm',
        )
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Gradient Norm (log scale)')
        ax.set_title('Gradient Evolution')
        ax.legend()
        ax.grid(True, alpha=0.3)

        plt.tight_layout()
        plt.show()


def main():
    """Run live visualization example"""

    print("=" * 70)
    print("Brimstone Live Visualization Example (PHASE 3 FEATURE)")
    print("=" * 70)
    print()
    print("NOTE: This example demonstrates the planned callback functionality.")
    print("      The callback mechanism will be fully implemented in Phase 3.")
    print("      Currently running basic optimization without live updates.")
    print()

    # Create plan and optimizer
    print("Setting up treatment plan...")
    plan = pb.Plan()

    beam_angles = [0, 45, 90, 135, 180, 225, 270, 315]
    for angle in beam_angles:
        beam = pb.Beam(gantry_angle=angle)
        print(f"  Added beam at {angle}°")

    optimizer = pb.PlanOptimizer(plan)
    print(f"Created optimizer with {plan.beam_count} beams")
    print()

    # Create visualization callback
    print("Creating visualization callback...")
    viz_callback = LiveVisualizationCallback(update_interval=10)
    # viz_callback.setup_plots()  # Uncomment when callback mechanism is ready
    print()

    # Run optimization
    print("Running optimization...")
    print("  (Callback updates will be available in Phase 3)")
    result = optimizer.optimize(
        # callback=viz_callback,  # Uncomment when callback mechanism is ready
        max_iterations=500,
        tolerance=1e-3,
    )
    print()

    # Display results
    print("Optimization completed!")
    print(f"  Success: {result['success']}")
    print(f"  Iterations: {result['iterations']}")
    print(f"  Final cost: {result['final_cost']:.6f}")
    print()

    # Plot final results (if callback was used)
    # viz_callback.plot_final_results()  # Uncomment when callback mechanism is ready

    print("=" * 70)
    print("Example completed!")
    print("This will be fully functional in Phase 3 with callback support.")
    print("=" * 70)


if __name__ == "__main__":
    main()
