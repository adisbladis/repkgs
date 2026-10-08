// The two-key scheme shared by the C and rustc modes.
//   k1 = H(tool, cwd, normalised args, primary source bytes)   -> manifest: "input\tid" and "!path" lines
//   k2 = H(k1, manifest)                                        -> artifacts
// A manifest is valid when every input still has the same id (see Store::InputId) and every
// "!path" (a lookup the compiler made and missed) still does not exist. A compiler that does not
// report its missed lookups ends its manifest "#preprocessed": k2 then also folds the hash of the
// compile's preprocessed text, which shows every header the lookups resolved to.
#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "keys.h"

namespace jig {

class CacheClient;

// Prerequisites of the first rule of a make-style depfile. With -MP the compiler appends one
// phony "header:" rule per header. Those are not read.
auto ParseDepfile(std::string_view text) -> std::vector<std::string>;

struct Manifest {
  std::string text;  // "path\tid\n" per input, "!path\n" per absent path, store paths masked in content mode,
                     // then "#preprocessed\n" when k2 folds the preprocessed text
  ResultKey result_key;
};

// Both ask the daemon (one round trip) for the identities of store files first, so only build-tree
// inputs are hashed here. An unconnected client just means everything is hashed locally.

// one IDS round trip for the store files among `paths`. InputId then finds them without hashing
void PrefetchIdentities(CacheClient& cache, std::span<const std::string> paths);

// `inputs` minus `primary_source` (already in k1) and minus unreadable paths. `absent`: paths the
// compiler looked up and did not find, relative ones against the cwd. `preprocessed`: the hash of the
// preprocessed text, for a compiler that reports no missed lookups
auto BuildManifest(CacheClient& cache, const RequestKey& request_key, std::span<const std::string> inputs,
                   std::string_view primary_source, std::span<const std::string> absent = {},
                   const std::optional<std::string>& preprocessed = std::nullopt) -> Manifest;

// the hash of this run's preprocessed text, asked only by a manifest that ends "#preprocessed";
// nullopt when it cannot be made
using Preprocess = std::function<std::optional<std::string>()>;

// Recompute k2 from a stored manifest, or "inputs-changed:<path>" for the first input whose
// identity moved or vanished
auto ValidateManifest(CacheClient& cache, const RequestKey& request_key, std::string_view manifest_text,
                      const Preprocess& preprocess = {}) -> std::expected<ResultKey, std::string>;

// k1 -> manifest -> k2, or why not ("new-key" when k1 has no manifest)
auto FindResult(CacheClient& cache, const RequestKey& request_key, const Preprocess& preprocess = {})
    -> std::expected<ResultKey, std::string>;

}  // namespace jig
