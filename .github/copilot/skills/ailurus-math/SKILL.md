# Ailurus Math Library

## Scope
Templated math types (vectors, matrices, quaternions, euler angles), geometric primitives (AABB, frustum), random number generation, and rotation conversion utilities. All types interoperate with GLM.

## Key Files
- `include/Ailurus/Math/Vector2.hpp` — 2D vector
- `include/Ailurus/Math/Vector3.hpp` — 3D vector
- `include/Ailurus/Math/Vector4.hpp` — 4D vector
- `include/Ailurus/Math/Matrix2x2.hpp` — 2×2 matrix
- `include/Ailurus/Math/Matrix3x3.hpp` — 3×3 matrix
- `include/Ailurus/Math/Matrix4x4.hpp` — 4×4 matrix
- `include/Ailurus/Math/Quaternion.hpp` — Quaternion
- `include/Ailurus/Math/EulerAngle.hpp` — Euler angles (ZYX intrinsic)
- `include/Ailurus/Math/AABB.hpp` — Axis-Aligned Bounding Box
- `include/Ailurus/Math/Frustum.hpp` — View frustum (6 planes)
- `include/Ailurus/Math/Random.hpp` — Mersenne Twister RNG
- `include/Ailurus/Math/Math.hpp` / `Math.inl` — Conversion & utility functions
- `test/Math/` — Comprehensive unit tests

## Type Aliases Convention
All templated types provide aliases: `<Type>i` (int), `<Type>f` (float), `<Type>d` (double), `<Type>u` (unsigned int), `<Type>l` (long long), `<Type>ul` (unsigned long long).

Example: `Vector3f`, `Matrix4x4f`, `Quaternionf`, `EulerAnglesf`, `AABBf`.

## GLM Interoperability
Every type has `using GlmType = glm::...` and internal storage backed by GLM.
- `Glm()` → reference to underlying GLM type
- `GetDataPtr()` → column-major raw pointer (matrices)
- Constructors accept GLM types directly

## Vector Types (Vector2, Vector3, Vector4)

**Members:** `x`, `y`, [`z`, [`w`]]

**Constructors:** Default (zeros), value, copy, move. Explicit `operator Vector<T>()` for type conversion.

**Operations:**
- `SquareMagnitude()`, `Magnitude()` — Length
- `Normalize()` (in-place), `Normalized()` (copy) — Unit vector
- `Dot(other)` — Dot product
- `Cross(other)` — Cross product (Vector3 only)

**Vector3 Static Constants:** `Zero`, `One`, `Up(0,0,1)`, `Right(0,1,0)`, `Forward(1,0,0)`

**Vector3 Static Methods:** `Min(a,b)`, `Max(a,b)` — Component-wise min/max

**Operators:** `+`, `-`, `*`, `/` with scalars and vectors; `+=`, `-=`, `*=`, `/=`; `==`, `!=`; unary `-`.

## Matrix Types (Matrix2x2, Matrix3x3, Matrix4x4)

**Constructors:** Default (zero), from GLM, from row vectors, from `initializer_list<VectorN>`, copy, move.

**Element Access:**
- `operator[](col)` — Column access (GLM convention, returns column vector)
- `operator()(row, col)` — Row-major element access
- `GetRow(i)`, `GetCol(j)`, `SetRow(i, vec)`, `SetCol(j, vec)`

**Operations:**
- `Determinant()`, `Transpose()`, `Adjugate()`, `Inverse()`

**Data:** `GetDataPtr()` → column-major float pointer. `Glm()` → GLM reference.

**Static Constants:** `Zero`, `Identity`.

**Operators:** `*` (matrix × matrix, matrix × vector, matrix × scalar); `/` scalar; `==`, `!=`.

## Quaternion

**Members:** `x, y, z, w` (w is scalar part). Default: identity (0,0,0,1).

**Constraint:** `requires is_floating_point_v<ElementType>`

**Operations:**
- `Dot(other)`, `Magnitude()`, `Normalize()`, `Normalized()`
- `Conjugate()` → `(-x,-y,-z,w)`
- `Inverse()` → `Conjugate() / Magnitude²`

**Operators:**
- `*` Quaternion × Quaternion — Hamilton product (non-commutative)
- `*` Quaternion × Vector3 — Rotate vector
- `+`, `-` — Element-wise. Scalar `*`. Unary `-`. `==`, `!=`.

**Aliases:** `Quaternionf`, `Quaterniond`.

## EulerAngles

**Members:** `pitch` (X-axis), `yaw` (Z-axis), `roll` (Y-axis). All in radians. ZYX intrinsic rotation order.

**Aliases:** `EulerAnglesf`, `EulerAnglesd`.

## Math Namespace Utilities (Math.hpp / Math.inl)

**Angle Conversion:**
- `DegreeToRadian(T deg)`, `RadianToDegree(T rad)`

**Matrix Construction:**
- `TranslateMatrix(Vector3)` → Matrix4x4
- `ScaleMatrix(Vector3)` → Matrix4x4
- `LookAtMatrix(forward, up)` → Matrix4x4
- `LookAtQuaternion(forward, up)` → Quaternion

**Rotation:**
- `RotateAxis(axis, angleDeg)` → Quaternion
- `EulerAngleToQuaternion(euler)` → Quaternion
- `QuaternionToEulerAngles(quat)` → EulerAngles
- `QuaternionToRotateMatrix(quat)` → Matrix4x4
- `RotateMatrixToQuaternion(mat)` → Quaternion
- `EulerAngleToRotateMatrix(euler)` → Matrix4x4
- `RotateMatrixToEulerAngle(mat)` → EulerAngles

**Interpolation:**
- `Lerp(a, b, t)` — Linear interpolation (generic)
- `SLerp(q1, q2, t)` — Spherical linear interpolation (quaternions)

## AABB (Axis-Aligned Bounding Box)

**Members:** `Vector3<T> min`, `Vector3<T> max`.

**Methods:**
- `GetCenter()` → midpoint
- `GetExtents()` → half-size
- `Transform(Matrix4x4)` → transformed AABB (recomputes min/max from 8 corners)

**Static:** `Merge(a, b)` → union AABB.

## Frustum

**FrustumPlane:** `Vector3f normal`, `float distance`. `SignedDistance(point)` → float.

**Frustum:** `FrustumPlane planes[6]` (Near, Far, Left, Right, Top, Bottom).
- `FromViewProjection(Matrix4x4f vp)` — Gribb-Hartmann plane extraction
- `Intersects(AABBf)` → bool — P-vertex/N-vertex test

## Random

**Members:** `mt19937 _generator` (Mersenne Twister).
Constructors: default (time seed), explicit seed.
- `Next<T>(min, max)` — Integral or floating-point random
- `NextBool(probability)` — Bernoulli
- `SetSeed(seed)`, `GetGenerator()` — Direct access
