/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_CC_PROFILER_PROFILER_H_
#define TENSORFLOW_CC_PROFILER_PROFILER_H_

#include <string>
#include <memory>

#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/profiler/internal/tfprof_stats.h"
#include "tensorflow/core/profiler/tfprof_options.h"
#include "tensorflow/core/profiler/tfprof_output.pb.h"

namespace tensorflow {
namespace tfprof {

/// @addtogroup core
/// @{

/// A `Profiler` object lets the caller profile the execution of a graph.
///
/// Example:
///     // First build a graph and run tracing.
///     Scope root = Scope::NewRootScope();
///     auto a = Placeholder(root, DT_INT32);
///     auto c = Add(root, a, {41});
///
///     ClientSession session(root);
///     std::vector<Tensor> outputs;
///     RunOptions run_options;
///     run_options.set_trace_level(RunOptions::FULL_TRACE);
///     RunMetadata run_meta;
///     Status s = session.Run(run_options, { {a, {1}} }, {c}, &outputs,
///                            &run_meta);
///     if (!s.ok()) { ... }
///
///     // Then create profiler to do profiling.
///     GraphDef graph;
///     root.ToGraphDef(&graph);
///     Profiler profiler(graph);
///     profiler.AddStep(0, run_meta);
///     Options opts = ...  // TODO(xpan): Support option building API.
///     MultiGraphNodeProto r = profiler.ProfileOperations(opts);
///
class Profiler {
 public:
  /// `graph` is the model's GraphDef.
  explicit Profiler(const GraphDef& graph);

  /// Adds tracing information `run_meta` to profiler. A `run_meta` is
  /// generated by a TensorFlow session run call. `step` is the key
  /// to the `run_meta`. When calling ProfileXXX methods, caller can specify
  /// `step` in `options` to selectively profile the corresponding `run_meta`.
  /// Multiple different `run_meta` can be keyed by the same `step` in order
  /// to group them together.
  void AddStep(int64 step, const RunMetadata& run_meta);

  /// Profiles the model by organizing nodes in graph structure.
  /// Each node is an op and the nodes are connected by the op inputs/outputs.
  GraphNodeProto ProfileGraph(const Options& options);

  /// Profiles the model by organizing nodes in name scope structure.
  /// Each node is an op, and nodes are organized by the ops' name
  /// scope, similar to a file system tree.
  /// E.g. /foo is the root of operation /foo/matmul_1 and foo/conv_2.
  GraphNodeProto ProfileNameScope(const Options& options);

  /// Profiles the model by organizing nodes by operation types.
  /// Each node is an operation type (e.g. Conv2D or MatMul), containing all
  /// ops belonging to that type in the model.
  MultiGraphNodeProto ProfileOperations(const Options& options);

  /// Serialize the profile content (ProfileProto) into a binary string,
  /// User can write the string to file for offline analysis by
  /// tfprof command-line tools or graphical user interface.
  Status SerializeToString(string* content);

 private:
  std::unique_ptr<TFStats> stats_;
};
/// @}

}  // namespace tfprof
}  // namespace tensorflow

#endif  // TENSORFLOW_CC_PROFILER_PROFILER_H_
