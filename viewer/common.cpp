#include "common.hpp"


namespace isis
{
namespace viewer
{

void setOrientationToIdentity( data::Image &image )
{
	image.setPropertyAs<isis::util::fvector4>( "rowVec", isis::util::fvector4( 1, 0, 0 ) );
	image.setPropertyAs<isis::util::fvector4>( "columnVec", isis::util::fvector4( 0, 1, 0 ) );
	image.setPropertyAs<isis::util::fvector4>( "sliceVec", isis::util::fvector4( 0, 0, 1 ) );
	image.setPropertyAs<isis::util::fvector4>( "indexOrigin", isis::util::fvector4( 0, 0, 0 ) );
}

}
}