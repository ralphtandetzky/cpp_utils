#include "progress.h"

#include "locking.h"
#include "std_make_unique.h"

#include <vector>

namespace cu {

struct ParallelProgress::Impl
{
    class ParallelPartialProgress;

    Impl( ProgressInterface & parent,
          size_t nTasks,
          size_t nWorkers );

    ProgressInterface & parent;
    const size_t nWorkers;
    std::vector<ParallelPartialProgress> progressInterfaces;
    struct Shared
    {
        std::vector<double> progressVals;
        // progressVals[sortMapping[i]] is ascending with i
        std::vector<size_t> sortMapping;
        // sortMapping[inverseSortMapping[i]] == i
        std::vector<size_t> inverseSortMapping;
    };
    cu::Monitor<Shared> shared;
};

class ParallelProgress::Impl::ParallelPartialProgress
        : public ProgressInterface
{
public:
    ParallelPartialProgress( Impl * impl, size_t index )
        : index(index)
        , impl(impl)
    {}

    virtual void setProgress( double progress )
    {
        assert( progress >= 0 );
        assert( progress <= 1 );

        impl->shared( [&]( Shared & shared ){
            auto & progressVals = shared.progressVals;
            auto & mapping = shared.sortMapping;
            auto & inverse = shared.inverseSortMapping;
            assert( progress >= progressVals[index] );
            progressVals[index] = progress;
            const auto nTasks = progressVals.size();
            auto sortedIndex = inverse[index];
            while ( sortedIndex > 0 &&
                    progress > progressVals[mapping[sortedIndex-1]] )
            {
                using namespace std;
                swap( inverse[mapping[sortedIndex]],
                      inverse[mapping[sortedIndex-1]] );
                swap( mapping[sortedIndex], mapping[sortedIndex-1] );
                --sortedIndex;
            }
            const auto nWorkers = impl->nWorkers;
            if ( (nTasks - sortedIndex) % nWorkers != 0 )
                return;
            const auto nTotalChunks      = (nTasks + nWorkers-1 ) / nWorkers;
            const auto nIncompleteChunks = (nTasks - sortedIndex) / nWorkers + 1;
            const auto nCompleteChunks   = nTotalChunks - nIncompleteChunks;
            impl->parent.setProgress( (nCompleteChunks+progress)/nTotalChunks );
        } );
    }

    virtual bool shallAbort() const
    {
        return impl->parent.shallAbort();
    }

private:
    const size_t index = 0;
    Impl * const impl = nullptr;
};

ParallelProgress::Impl::Impl( ProgressInterface & parent,
      size_t nTasks,
      size_t nWorkers )
    : parent(parent)
    , nWorkers(nWorkers)
{
    shared( [&]( Shared & shared )
    {
        shared.sortMapping.reserve(nTasks);
        for ( size_t i = 0; i != nTasks; ++i )
        {
            progressInterfaces.push_back(
                ParallelProgress::Impl::ParallelPartialProgress( this, i ) );
            shared.sortMapping.push_back(i);
        }
        shared.inverseSortMapping = shared.sortMapping;
        shared.progressVals.assign( nTasks, 0. );
    } );
}

ParallelProgress::ParallelProgress(
        ProgressInterface & parent,
        size_t nTasks,
        size_t nWorkers )
    : m( std::make_unique<Impl>(parent,nTasks,nWorkers) )
{
}

ParallelProgress::~ParallelProgress() = default;

ProgressInterface & ParallelProgress::getTaskProgressInterface(
        size_t taskIndex ) const
{
    return m->progressInterfaces[taskIndex];
}

} // namespace cu
