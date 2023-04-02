// Copyright (c) 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CORE_PMTRACE_PROVIDER_LIB_WRAPPER_H_
#define CORE_PMTRACE_PROVIDER_LIB_WRAPPER_H_

#include <string>

namespace pmtrace {
void TraceMessage(const char* label);
void TraceBefore(const char* label);
void TraceAfter(const char* label);
void TraceScopeEntry(const char* label);
void TraceScopeExit(const char* label);
void TraceFunctionEntry(const char* label);
void TraceFunctionExit(const char* label);
void TraceItem(const char* name, const char* value);
void TracePosition(const char* label, int pos_x, int pos_y);
}  // namespace pmtrace

#endif  // CORE_PMTRACE_PROVIDER_LIB_WRAPPER_H_
