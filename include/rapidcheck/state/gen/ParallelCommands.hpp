#pragma once

#include "rapidcheck/state/gen/Commands.h"

namespace rc {
namespace state {
namespace gen {

namespace detail {
template <typename Cmd, typename GenFunc>
class ParallelCommandsGen {
public:
  using CmdSP = std::shared_ptr<const Cmd>;
  using Model = typename Cmd::Model;
  using Sut = typename Cmd::Sut;

  template <typename ModelArg, typename GenFuncArg>
  ParallelCommandsGen(ModelArg &&initialState, GenFuncArg &&genFunc)
      : m_initialState(std::forward<ModelArg>(initialState))
      , m_genFunc(std::forward<GenFuncArg>(genFunc)) {}

  Shrinkable<ParallelCommands<Cmd>> operator()(const Random &random,
                                               int size) const {
    return generateCommands(random, size);
  }

private:
  struct CommandEntry {
    CommandEntry(Random &&aRandom, Shrinkable<CmdSP> &&aShrinkable)
        : random(std::move(aRandom))
        , shrinkable(std::move(aShrinkable)) {}

    Random random;
    Shrinkable<CmdSP> shrinkable;
  };

  struct CommandSequence {
    CommandSequence(const GenFunc &func, int sz)
        : genFunc(func)
        , size(sz) {}

    Model initialState;
    GenFunc genFunc;
    int size;
    std::vector<CommandEntry> entries;


    void repairEntriesFrom(std::size_t start) {
      for (auto i = start; i < entries.size(); i++) {
        if (!repairEntryAt(i)) {
          entries.erase(begin(entries) + i--);
        }
      }
    }

    bool repairEntryAt(std::size_t i) {
      using namespace ::rc::detail;
      try {
        auto &entry = entries[i];
        const auto cmd = entry.shrinkable.value();
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }

        return regenerateEntryAt(i);
      }

      return true;
    }

    bool regenerateEntryAt(std::size_t i) {
      using namespace ::rc::detail;
      try {
        auto &entry = entries[i];
        entry.shrinkable = genFunc()(entry.random, size);
        const auto cmd = entry.shrinkable.value();
        return true;
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }
      } catch (const GenerationFailure &failure) {
        // Just return false below
      }

      return false;
    }
  };

  struct ParallelCommandSequence {
    ParallelCommandSequence(CommandSequence prefix,
                            CommandSequence left,
                            CommandSequence right)
        : prefix(prefix)
        , left(left)
        , right(right) {}

    CommandSequence prefix;
    CommandSequence left;
    CommandSequence right;
  };

  Shrinkable<ParallelCommands<Cmd>> generateCommands(const Random &random,
                                                     int size) const {
    return shrinkable::map(generateParallelSequence(random, size),
                           [](const ParallelCommandSequence &sequence) {
                             ParallelCommands<Cmd> cmds;
                             copyCommandEntries(sequence.prefix, cmds.prefix);
                             copyCommandEntries(sequence.left, cmds.left);
                             copyCommandEntries(sequence.right, cmds.right);
                             return cmds;
                           });
  }

  template <typename T>
  static void copyCommandEntries(const CommandSequence &source,
                                 std::vector<T> &dest) {
    const auto &entries = source.entries;
    dest.reserve(entries.size());
    std::transform(
        begin(entries),
        end(entries),
        std::back_inserter(dest),
        [](const CommandEntry &entry) { return entry.shrinkable.value(); });
  }

  Shrinkable<ParallelCommandSequence>
  generateParallelSequence(const Random &random, int size) const {
    Random r(random);
    std::size_t count = (r.split().next() % (size + 1)) + 1;
    return shrinkable::shrinkRecur(generateInitialParallel(random, size),
                                   &shrinkSequence);
  }

  ParallelCommandSequence generateInitialParallel(const Random &random,
                                                  int size) const {
    auto r = random;
    int prefixSz, leftSz, rightSz;
    std::tie(prefixSz, leftSz, rightSz) = parallelCommandDistribution(size);

    std::size_t prefixCount = (r.split().next() % (prefixSz + 1));
    std::size_t leftCount = (r.split().next() % (leftSz + 1));
    std::size_t rightCount = (r.split().next() % (rightSz + 1));

    return ParallelCommandSequence(
        generateInitial(random, prefixSz, prefixCount),
        generateInitial(random, leftSz, leftCount),
        generateInitial(random, rightSz, rightCount));
  }

  Shrinkable<CommandSequence> generateSequence(const Random &random,
                                               int size) const {
    Random r(random);
    std::size_t count = (r.split().next() % (size + 1)) + 1;
    return shrinkable::shrinkRecur(generateInitial(random, size, count),
                                   &shrinkSequence);
  }

  CommandSequence
  generateInitial(const Random &random, int size, std::size_t count) const {
    CommandSequence sequence(m_genFunc, size);
    sequence.entries.reserve(count);

    auto r = random;
    while (sequence.entries.size() < count) {
      sequence.entries.push_back(nextEntry(r.split(), size));
    }

    return sequence;
  }

  CommandEntry nextEntry(const Random &random, int size) const {
    using namespace ::rc::detail;
    auto r = random;
    const auto gen = m_genFunc();
    // TODO configurability?
    for (int tries = 0; tries < 100; tries++) {
      try {
        auto random = r.split();
        auto shrinkable = gen(random, size);

        return CommandEntry(std::move(random), std::move(shrinkable));
      } catch (const CaseResult &result) {
        if (result.type != CaseResult::Type::Discard) {
          throw;
        }
        // What to do?
      } catch (const GenerationFailure &failure) {
        // What to do?
      }
    }

    // TODO better error message
    throw GenerationFailure("Failed to generate command after 100 tries.");
  }

  static Seq<ParallelCommandSequence>
  shrinkSequence(const ParallelCommandSequence &s) {
    return seq::concat(shrinkPrefix(s),
                       shrinkLeft(s),
                       shrinkRight(s),
                       unparallelizeLeft(s),
                       unparallelizeRight(s));
  }

  static Seq<ParallelCommandSequence>
  unparallelizeLeft(const ParallelCommandSequence &s) {
    auto valid = seq::filter(seq::just(s),
                             [](const ParallelCommandSequence &s) {
                               return !s.left.entries.empty();
                             });

    return seq::map(std::move(valid),
                    [](const ParallelCommandSequence &s) {
                      auto prefix = s.prefix;
                      auto left = s.left;
                      auto head = left.entries.begin();
                      prefix.entries.push_back(*head);
                      left.entries.erase(head);
                      return ParallelCommandSequence(prefix, left, s.right);
                    });
  }

  static Seq<ParallelCommandSequence>
  unparallelizeRight(const ParallelCommandSequence &s) {
    auto valid = seq::filter(seq::just(s),
                             [](const ParallelCommandSequence &s) {
                               return !s.right.entries.empty();
                             });

    return seq::map(std::move(valid),
                    [](const ParallelCommandSequence &s) {
                      auto prefix = s.prefix;
                      auto right = s.right;
                      auto head = right.entries.begin();
                      prefix.entries.push_back(*head);
                      right.entries.erase(head);
                      return ParallelCommandSequence(prefix, s.left, right);
                    });
  }

  static Seq<ParallelCommandSequence>
  shrinkPrefix(const ParallelCommandSequence &s) {
    auto shrunkSeqs =
        seq::concat(shrinkRemoving(s.prefix), shrinkIndividual(s.prefix));
    return seq::map(std::move(shrunkSeqs),
                    [=](const CommandSequence &commands) {
                      return ParallelCommandSequence(commands, s.left, s.right);
                    });
  }

  static Seq<ParallelCommandSequence>
  shrinkLeft(const ParallelCommandSequence &s) {
    auto shrunkSeqs =
        seq::concat(shrinkRemoving(s.left), shrinkIndividual(s.left));
    return seq::map(std::move(shrunkSeqs),
                    [=](const CommandSequence &commands) {
                      return ParallelCommandSequence(
                          s.prefix, commands, s.right);
                    });
  }

  static Seq<ParallelCommandSequence>
  shrinkRight(const ParallelCommandSequence &s) {
    auto shrunkSeqs =
        seq::concat(shrinkRemoving(s.right), shrinkIndividual(s.right));
    return seq::map(std::move(shrunkSeqs),
                    [=](const CommandSequence &commands) {
                      return ParallelCommandSequence(
                          s.prefix, s.left, commands);
                    });
  }

  static Seq<CommandSequence> shrinkRemoving(const CommandSequence &s) {
    auto nonEmptyRanges = seq::subranges(0, s.entries.size());
    return seq::map(std::move(nonEmptyRanges),
                    [=](const std::pair<std::size_t, std::size_t> &r) {
                      auto shrunk = s;
                      shrunk.entries.erase(begin(shrunk.entries) + r.first,
                                           begin(shrunk.entries) + r.second);
                      shrunk.repairEntriesFrom(r.first);
                      return shrunk;
                    });
  }

  static Seq<CommandSequence> shrinkIndividual(const CommandSequence &s) {
    return seq::mapcat(seq::range<std::size_t>(0, s.entries.size()),
                       [=](std::size_t i) {
                         return seq::map(s.entries[i].shrinkable.shrinks(),
                                         [=](Shrinkable<CmdSP> &&cmd) {
                                           auto shrunk = s;
                                           auto &entry = shrunk.entries[i];

                                           entry.shrinkable = std::move(cmd);
                                           shrunk.repairEntriesFrom(i);

                                           return shrunk;
                                         });
                       });
  }

  /// Calculates how many commands to generate for each subsequence.
  /// Returns a three tuple with the number of commands for {prefix,
  /// first parallel sequence, second parallel sequence}
  static std::tuple<int, int, int> parallelCommandDistribution(int cmdCount) {
    if (cmdCount < 12) {
      // If there are fewer than 12 commands, make all parallel
      return std::make_tuple(0, cmdCount / 2, cmdCount - cmdCount / 2);
    } else {
      // If there are more than 12 commands, make the 12 last ones parallel
      return std::make_tuple(cmdCount - 12, 6, 6);
    }
  }

  Model m_initialState;
  GenFunc m_genFunc;
};

} // detail

template <typename Cmd, typename GenerationFunc>
Gen<ParallelCommands<Cmd>>
parallelCommands(const typename Cmd::Model &initialState,
                 GenerationFunc &&genFunc) {
  return detail::ParallelCommandsGen<Cmd, Decay<GenerationFunc>>(
      initialState, std::forward<GenerationFunc>(genFunc));
}

} // namespace gen
} // namespace state

template <typename Cmd>
struct ShowType<state::ParallelCommands<Cmd>> {
  static void showType(std::ostream &os) {
    os << "Parallel command sequence ";
    detail::showType<Cmd>(os);
  }
};

} // namespace rc
