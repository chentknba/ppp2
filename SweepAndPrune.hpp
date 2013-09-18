#ifndef SWEEPANDPRUNE_HPP
#define SWEEPANDPRUNE_HPP

#include "GridSuperstructure.hpp"
#include "SegSNP.hpp"


namespace SweepAndPrune
{
	struct NoThreads 
	{
		typedef int threadIdType;
		static void Create( threadIdType * id, void *(*routine)(void *), void * userData)
		{
			(*routine)(userData);
		}

		static void Join( threadIdType id ) {}
	};
	

	template<typename Key>
	struct FilterNone 
	{
		bool pass( Key one, Key two ) { return true; }
	};
	

	template<int D, typename ClientKey, typename Coordinate = float, class ThreadPolicy = NoThreads, class FilterPolicy = FilterNone<ClientKey> >
	class SweepAndPrune : public FullOutputAdapter<GridSubdivider<Segmented, false, D, ClientKey, Coordinate, ThreadPolicy, FilterPolicy> >
	{
	};
}

#endif