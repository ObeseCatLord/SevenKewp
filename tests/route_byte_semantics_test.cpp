#include "extdll.h"
#include "util.h"
#include "monster/nodes.h"

#include <assert.h>

int main()
{
	const int sequenceCounts[] = { 1, 2, 127, 128 };
	for (int i = 0; i < sizeof(sequenceCounts) / sizeof(sequenceCounts[0]); ++i)
	{
		const int count = sequenceCounts[i];
		assert(RouteSequenceLength(RouteSequenceByte(count)) == count);
	}
	assert(RouteSequenceByte(1) == 0xff);
	assert(RouteSequenceByte(128) == 0x80);

	const int deltas[] = { -128, -127, -1, 0, 1, 127 };
	for (int i = 0; i < sizeof(deltas) / sizeof(deltas[0]); ++i)
	{
		const int delta = deltas[i];
		assert(RouteByteToDelta(RouteDeltaByte(delta)) == delta);
	}
	assert(RouteDeltaByte(-128) == 0x80);
	assert(RouteDeltaByte(-1) == 0xff);

	return 0;
}
