# Hyperbolic Riemannian Optimization & Trajectory Indexing Engine

A high-performance experimental simulator and low-latency storage engine designed to trace and index optimization trajectories on non-Euclidean manifolds. This system models optimization particles navigating the **Lorentz Hyperboloid Model** ($\mathbb{H}^n$) using Riemannian Gradient Descent (RGD), compressing and indexing their spatio-temporal path states to evaluate convergence and divergence profiles in real time.

The core architecture prioritizes zero-allocation execution paths, explicit CPU cache-line alignment, and hardware-level compute density.

---

## Architectural Foundations

### 1. The Mathematical Domain: The Lorentz Model

Unlike the bounded Poincaré Disk, which introduces severe coordinate-induced gradient vanishing at the boundaries, this engine globally embeds trajectories onto the unbounded forward sheet of a hyperboloid in a Minkowski space $\mathbb{R}^{n+1}$.

* **Manifold Invariant:** Every vector $x$ must strictly satisfy the Lorentz metric constraint:

$$\langle x, x \rangle_L = -x_0^2 + x_1^2 + \dots + x_n^2 = -1 \quad \text{where } x_0 > 0$$


* **Distance Metric:** Geodesic distance is computed via the Minkowski inner product:

$$d_{\mathbb{H}}(u, v) = \text{acosh}(-\langle u, v \rangle_L)$$



### 2. High-Performance Hardware Alignment

* **Contiguous Flat Allocations:** To completely bypass pointer-chasing and cache invalidations, the database rejects individual structural node allocations. All vector coordinates are stored in a singular, flat, pre-allocated memory buffer.
* **Cache-Line Boundary Constraints:** Vectors are explicitly padded so that their dimension footprint ($d+1$) perfectly matches hardware cache lines (64 bytes) or SIMD register boundaries (32/64 bytes), forcing predictable CPU pre-fetch behaviors.

---

## Engine Roadmap

### Phase 1: The Linear Baseline MVP (Current)

* [x] **Flat Layout Buffer:** A dense `std::vector<float>` sequence managed via `std::span` data windows.
* [ ] **Lorentz Math Kernel:** Pure, deterministic implementations of the Minkowski inner product, hyperbolic distance, and tangent space projections.
* [ ] **$O(N)$ Ground-Truth Reference Scanner:** A brute-force linear search suite used as the absolute correctness baseline to evaluate future lossy compression indices.
* [ ] **Trajectory Generation Harness:** A synthetic simulation driver running multi-particle Riemannian Gradient Descent (RGD) to populate tracking paths.

### Phase 2: Hardware Acceleration & Lossy Quantization (Upcoming)

* [ ] **AVX2 / AVX-512 SIMD Mathematical Overrides:** Vectorized Taylor-series approximations of transcendental functions ($\text{acosh}$, $\text{exp}$) to eliminate inner-loop execution blocks.
* [ ] **Minkowski Inverted File Product Quantization (IVF-PQ):** Spatial partitioning and subspace quantization modified explicitly for Minkowski space metrics, reducing memory footprints by over 90%.

---

## Experimental Telemetry Suite

The system includes an integrated micro-benchmarking and mathematical validation engine designed to profile:

1. **Topological Recall Accuracy:** Measuring the empirical Mean Squared Error (MSE) of quantized distance approximations against the exact $O(N)$ linear reference scanner.
2. **Hardware Latency Metrics:** Tracking nanosecond-level processing speed, cache miss frequencies, and instructions-per-cycle (IPC) across varying dimensional granularities ($d=16$ up to $d=1024$).

---

## Build & Environmental Requirements

* **Compiler:** Clang++ or G++ supporting full **C++20** standard configurations.
* **Build System:** CMake 3.20+
* **Hardware Targets:** x86_64 architecture with AVX2 or AVX-512 capability.
