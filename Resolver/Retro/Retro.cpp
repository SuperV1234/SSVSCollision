#include "Retro.h"

using namespace std;
using namespace sf;
using namespace ssvsc::Utils;

namespace ssvsc
{
	void Retro::resolve(Body& mBody, std::vector<Body*>& mBodiesToResolve)
	{
		AABB& shape(mBody.getShape());
		const AABB& oldShape(mBody.getOldShape());
		sort(mBodiesToResolve, [&](Body* mA, Body* mB){ return getOverlapArea(shape, mA->getShape()) > getOverlapArea(shape, mB->getShape()); });

		for(auto& b : mBodiesToResolve)
		{
			const AABB& s(b->getShape()), os(b->getOldShape());
			Vector2i resolution{getMin1DIntersection(shape, s)};
			mBody.onResolution({*b, b->getUserData(), resolution});
			shape.move(resolution);

			if(resolution.y < 0)
			{
				if(oldShape.isAbove(s) || (os.isBelow(shape) && !(oldShape.isLeftOf(s) || oldShape.isRightOf(s))))
					if(mBody.getVelocity().y > 0) mBody.setVelocityY(0);
			}
			else if(resolution.y > 0)
			{
				if(oldShape.isBelow(s) || (os.isAbove(shape) && !(oldShape.isLeftOf(s) || oldShape.isRightOf(s))))
					if(mBody.getVelocity().y < 0) mBody.setVelocityY(0);
			}

			if(resolution.x < 0)
			{
				if(oldShape.isLeftOf(s) || (os.isRightOf(shape) && !(oldShape.isAbove(s) || oldShape.isBelow(s))))
					if(mBody.getVelocity().x > 0) mBody.setVelocityX(0);
			}
			else if(resolution.x > 0)
			{
				if(oldShape.isRightOf(s) || (os.isLeftOf(shape) && !(oldShape.isAbove(s) || oldShape.isBelow(s))))
					if(mBody.getVelocity().x < 0) mBody.setVelocityX(0);
			}
		}
	}
}

