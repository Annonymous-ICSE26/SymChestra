//===-- Searcher.h ----------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_SEARCHER_H
#define KLEE_SEARCHER_H

#include "klee/Internal/System/Time.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <queue>
#include <set>
#include <vector>
#include <unordered_map>
#include <regex>
#include <bitset>

namespace llvm {
  class BasicBlock;
  class Function;
  class Instruction;
  class raw_ostream;
}
namespace stats{
  extern bool currentUpdated;
}

namespace klee {
  template<class T> class DiscretePDF;
  class ExecutionState;
  class Executor;

  class Searcher {
  public:
    virtual ~Searcher();

    virtual ExecutionState &selectState() = 0;

    virtual void update(ExecutionState *current,
                        const std::vector<ExecutionState *> &addedStates,
                        const std::vector<ExecutionState *> &removedStates) = 0;

    virtual bool empty() = 0;

    // prints name of searcher as a klee_message()
    // TODO: could probably make prettier or more flexible
    virtual void printName(llvm::raw_ostream &os) {
      os << "<unnamed searcher>\n";
    }

    // pgbovine - to be called when a searcher gets activated and
    // deactivated, say, by a higher-level searcher; most searchers
    // don't need this functionality, so don't have to override.
    virtual void activate() {}
    virtual void deactivate() {}

    // utility functions

    void addState(ExecutionState *es, ExecutionState *current = 0) {
      std::vector<ExecutionState *> tmp;
      tmp.push_back(es);
      update(current, tmp, std::vector<ExecutionState *>());
    }

    void removeState(ExecutionState *es, ExecutionState *current = 0) {
      std::vector<ExecutionState *> tmp;
      tmp.push_back(es);
      update(current, std::vector<ExecutionState *>(), tmp);
    }

    enum CoreSearchType {
      DFS,
      BFS,
      RandomState,
      RandomPath,
      NURS_CovNew,
      NURS_MD2U,
      NURS_Depth,
      NURS_RP,
      NURS_ICnt,
      NURS_CPICnt,
      NURS_QC,
      AutoFeat,
    };
  };

  class DFSSearcher : public Searcher {
    std::vector<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "DFSSearcher\n";
    }
  };

  class BFSSearcher : public Searcher {
    std::deque<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "BFSSearcher\n";
    }
  };

  class RandomSearcher : public Searcher {
    std::vector<ExecutionState*> states;

  public:
    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return states.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "RandomSearcher\n";
    }
  };

  class WeightedRandomSearcher : public Searcher {
  public:
    enum WeightType {
      Depth,
      RP,
      QueryCost,
      InstCount,
      CPInstCount,
      MinDistToUncovered,
      CoveringNew
    };

  private:
    DiscretePDF<ExecutionState*> *states;
    WeightType type;
    bool updateWeights;
    
    double getWeight(ExecutionState*);

  public:
    WeightedRandomSearcher(WeightType type);
    ~WeightedRandomSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty();
    void printName(llvm::raw_ostream &os) {
      os << "WeightedRandomSearcher::";
      switch(type) {
      case Depth              : os << "Depth\n"; return;
      case RP                 : os << "RandomPath\n"; return;
      case QueryCost          : os << "QueryCost\n"; return;
      case InstCount          : os << "InstCount\n"; return;
      case CPInstCount        : os << "CPInstCount\n"; return;
      case MinDistToUncovered : os << "MinDistToUncovered\n"; return;
      case CoveringNew        : os << "CoveringNew\n"; return;
      default                 : os << "<unknown type>\n"; return;
      }
    }
  };

  class RandomPathSearcher : public Searcher {
    Executor &executor;

  public:
    RandomPathSearcher(Executor &_executor);
    ~RandomPathSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty();
    void printName(llvm::raw_ostream &os) {
      os << "RandomPathSearcher\n";
    }
  };


  extern llvm::cl::opt<bool> UseIncompleteMerge;
  class MergeHandler;
  class MergingSearcher : public Searcher {
    friend class MergeHandler;

    private:

    Executor &executor;
    Searcher *baseSearcher;

    public:
    MergingSearcher(Executor &executor, Searcher *baseSearcher);
    ~MergingSearcher();

    ExecutionState &selectState();

    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates) {
      baseSearcher->update(current, addedStates, removedStates);
    }
    bool empty() { return baseSearcher->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "MergingSearcher\n";
    }
  };
  
  class BatchingSearcher : public Searcher {
    Searcher *baseSearcher;
    time::Span timeBudget;
    unsigned instructionBudget;

    ExecutionState *lastState;
    time::Point lastStartTime;
    unsigned lastStartInstructions;

  public:
    BatchingSearcher(Searcher *baseSearcher, 
                     time::Span _timeBudget,
                     unsigned _instructionBudget);
    ~BatchingSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return baseSearcher->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "<BatchingSearcher> timeBudget: " << timeBudget
         << ", instructionBudget: " << instructionBudget
         << ", baseSearcher:\n";
      baseSearcher->printName(os);
      os << "</BatchingSearcher>\n";
    }
  };

  class IterativeDeepeningTimeSearcher : public Searcher {
    Searcher *baseSearcher;
    time::Point startTime;
    time::Span time;
    std::set<ExecutionState*> pausedStates;

  public:
    IterativeDeepeningTimeSearcher(Searcher *baseSearcher);
    ~IterativeDeepeningTimeSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return baseSearcher->empty() && pausedStates.empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "IterativeDeepeningTimeSearcher\n";
    }
  };

  class InterleavedSearcher : public Searcher {
    typedef std::vector<Searcher*> searchers_ty;

    searchers_ty searchers;
    unsigned index;

  public:
    explicit InterleavedSearcher(const searchers_ty &_searchers);
    ~InterleavedSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty() { return searchers[0]->empty(); }
    void printName(llvm::raw_ostream &os) {
      os << "<InterleavedSearcher> containing "
         << searchers.size() << " searchers:\n";
      for (searchers_ty::iterator it = searchers.begin(), ie = searchers.end();
           it != ie; ++it)
        (*it)->printName(os);
      os << "</InterleavedSearcher>\n";
    }
  };

  //FeatMaker
   class AutoFeatureSearcher : public Searcher {
  private:
    std::vector<ExecutionState*> states;
    Executor &executor;
    ExecutionState* selectedState;
    bool updateFeatureMap;
    std::unordered_map<std::string, int> feature_map;
    std::map<int, bool> EncounteredFeature;
    // std::map<ExecutionState*, std::vector<int>> FeatureEncounterCount;
    std::vector<double> weights;
    std::map<ExecutionState*, double> EsWeightMap;
    unsigned totalFeatureMaps;
    double getWeight(ExecutionState*);
    int featureCount;
    std::regex largeValueRe;
    std::regex constArrRe;
    std::string abstractCond(std::string line);

  public:
    AutoFeatureSearcher(const std::string &weightFile,
                          const std::string &featureFile,
                          Executor &_executor);
    ~AutoFeatureSearcher();

    ExecutionState &selectState();
    void update(ExecutionState *current,
                const std::vector<ExecutionState *> &addedStates,
                const std::vector<ExecutionState *> &removedStates);
    bool empty();
    void printName(llvm::raw_ostream &os) {
      os << "Automated feature searcher\n";
      }
  };

 //For Learch
  class BranchingSearcher : public Searcher {
    public:
      BranchingSearcher(Searcher *baseSearcher, Executor &_executor);
      ~BranchingSearcher();

      ExecutionState &selectState();
      void update(ExecutionState *current,
                  const std::vector<ExecutionState *> &addedStates,
                  const std::vector<ExecutionState *> &removedStates);
      bool empty() { return baseSearcher->empty(); }
      void printName(llvm::raw_ostream &os) {
          os << "<BranchingSearcher>"
             << ", baseSearcher:\n";
          baseSearcher->printName(os);
          os << "</BranchingSearcher>\n";
      }

      Searcher *baseSearcher;

    private:
      ExecutionState *lastState;
      unsigned lastSelectStackSize;
      Executor &executor;
  };
}

#endif /* KLEE_SEARCHER_H */
