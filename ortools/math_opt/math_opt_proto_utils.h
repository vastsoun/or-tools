// Copyright 2010-2021 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_MATH_OPT_MATH_OPT_PROTO_UTILS_H_
#define OR_TOOLS_MATH_OPT_MATH_OPT_PROTO_UTILS_H_

#include <cstdint>

#include "absl/container/flat_hash_set.h"
#include "ortools/base/integral_types.h"
#include "ortools/base/logging.h"
#include "ortools/math_opt/callback.pb.h"
#include "ortools/math_opt/model.pb.h"
#include "ortools/math_opt/sparse_containers.pb.h"

namespace operations_research {
namespace math_opt {

inline int NumVariables(const VariablesProto& variables) {
  return variables.ids_size();
}

inline int NumConstraints(const LinearConstraintsProto& linear_constraints) {
  return linear_constraints.ids_size();
}

inline int NumMatrixNonzeros(const SparseDoubleMatrixProto& matrix) {
  return matrix.row_ids_size();
}

// Removes the items in the sparse double vector for all indices whose value is
// exactly 0.0.
//
// NaN values are kept in place.
//
// The function asserts that input is a valid sparse vector, i.e. that the
// number of values and ids match.
void RemoveSparseDoubleVectorZeros(SparseDoubleVectorProto& sparse_vector);

// A utility class that tests if a pair (id, value) should be filtered based on
// an input SparseVectorFilterProto.
//
// This predicate expects the input is sorted by ids. In non-optimized builds,
// it will check that this is the case.
class SparseVectorFilterPredicate {
 public:
  // Builds a predicate based on the input filter. A reference to this filter is
  // kept so the caller must make sure this filter outlives the predicate.
  //
  // The filter.filtered_ids is expected to be sorted and not contain
  // duplicates. In non-optimized builds, it will be CHECKed.
  explicit SparseVectorFilterPredicate(const SparseVectorFilterProto& filter);

  // Returns true if the input value should be kept, false if it should be
  // ignored since it is not selected by the filter.
  //
  // This function is expected to be called with strictly increasing ids. In
  // non-optimized builds it will CHECK that this is the case. It updates an
  // internal counter when filtering by ids.
  template <typename Value>
  bool AcceptsAndUpdate(const int64_t id, const Value& value);

 private:
  const SparseVectorFilterProto& filter_;

  // Index of the next element to consider in filter_.filtered_ids().
  int next_filtered_id_index_ = 0;

#ifndef NDEBUG
  // Invariant: next input id must be >= next_input_id_lower_bound_.
  //
  // The initial value is 0 since all ids are expected to be non-negative.
  int next_input_id_lower_bound_ = 0;
#endif  // NDEBUG
};

// Returns the callback_registration.request_registration as a set of enums.
absl::flat_hash_set<CallbackEventProto> EventSet(
    const CallbackRegistrationProto& callback_registration);

////////////////////////////////////////////////////////////////////////////////
// Inline functions implementations.
////////////////////////////////////////////////////////////////////////////////

template <typename Value>
bool SparseVectorFilterPredicate::AcceptsAndUpdate(const int64_t id,
                                                   const Value& value) {
#ifndef NDEBUG
  CHECK_GE(id, next_input_id_lower_bound_)
      << "This function must be called with strictly increasing ids.";

  // Update the range of the next expected id. We expect input to be strictly
  // increasing.
  next_input_id_lower_bound_ = id + 1;
#endif  // NDEBUG

  // For this predicate we use `0` as the zero to test with since as of today we
  // only have SparseDoubleVectorProto and SparseBoolVectorProto. The `bool`
  // type is an integral type so the comparison with 0 will indeed be equivalent
  // to keeping only `true` values.
  if (filter_.skip_zero_values() && value == 0) {
    return false;
  }

  if (!filter_.filter_by_ids()) {
    return true;
  }

  // Skip all filtered_ids that are smaller than the input id.
  while (next_filtered_id_index_ < filter_.filtered_ids_size() &&
         filter_.filtered_ids(next_filtered_id_index_) < id) {
    ++next_filtered_id_index_;
  }

  if (next_filtered_id_index_ == filter_.filtered_ids_size()) {
    // We filter by ids and there are no more ids that should pass.
    return false;
  }

  // The previous loop ensured that the element at next_filtered_id_index_ is
  // the first element greater or equal to id.
  return id == filter_.filtered_ids(next_filtered_id_index_);
}

}  // namespace math_opt
}  // namespace operations_research

#endif  // OR_TOOLS_MATH_OPT_MATH_OPT_PROTO_UTILS_H_
