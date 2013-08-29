// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVSC_RESOLVER_IMPULSE
#define SSVSC_RESOLVER_IMPULSE

#include "SSVSCollision/Utils/Utils.h"
#include "SSVSCollision/Body/Body.h"
#include "SSVSCollision/Global/Typedefs.h"
#include "SSVSCollision/Resolver/ResolverBase.h"

namespace ssvsc
{
	class Body;

	struct Impulse : public ResolverBase
	{
		void resolve(float mFrameTime, Body& mBody, std::vector<Body*>& mBodiesToResolve) override
		{
			AABB& shape(mBody.getShape());
			const AABB& oldShape(mBody.getOldShape());
			ssvu::sort(mBodiesToResolve, [&](Body* mA, Body* mB){ return Utils::getOverlapArea(shape, mA->getShape()) > Utils::getOverlapArea(shape, mB->getShape()); });

			for(const auto& b : mBodiesToResolve)
			{
				const AABB& s(b->getShape());

				int iX{Utils::getMinIntersectionX(shape, s)}, iY{Utils::getMinIntersectionY(shape, s)};
				bool minAbs{std::abs(iX) < std::abs(iY)};
				bool noResolvePosition{false}, noResolveVelocity{false};
				Vec2i resolution{minAbs ? Vec2i{iX, 0} : Vec2i{0, iY}};
				mBody.onResolution({*b, b->getUserData(), {iX, iY}, resolution, noResolvePosition, noResolveVelocity});

				if(!noResolvePosition) shape.move(resolution);
				if(noResolveVelocity) continue;

				bool oldShapeLeftOfS{oldShape.isLeftOf(s)}, oldShapeRightOfS{oldShape.isRightOf(s)};
				bool oldShapeAboveS{oldShape.isAbove(s)}, oldShapeBelowS{oldShape.isBelow(s)};
				bool oldHOverlap{!(oldShapeLeftOfS || oldShapeRightOfS)}, oldVOverlap{!(oldShapeAboveS || oldShapeBelowS)};

				const auto& velocity(mBody.getVelocity());
				const AABB& os(b->getOldShape());
				float desiredX{velocity.x}, desiredY{velocity.y};

				if		(resolution.y < 0 && velocity.y > 0 && (oldShapeAboveS || (os.isBelow(shape) && oldHOverlap))) desiredY *= mBody.getRestitutionY();
				else if	(resolution.y > 0 && velocity.y < 0 && (oldShapeBelowS || (os.isAbove(shape) && oldHOverlap))) desiredY *= mBody.getRestitutionY();

				if		(resolution.x < 0 && velocity.x > 0 && (oldShapeLeftOfS || (os.isRightOf(shape) && oldVOverlap))) desiredX *= mBody.getRestitutionX();
				else if	(resolution.x > 0 && velocity.x < 0 && (oldShapeRightOfS || (os.isLeftOf(shape) && oldVOverlap))) desiredX *= mBody.getRestitutionX();

				Vec2f velDiff{b->getVelocity() - mBody.getVelocity()};
				Vec2f normal(ssvs::getNormalized(-resolution));

				float minDist{-(minAbs ? std::abs(iX) : std::abs(iY))};
				float remove{ssvs::getDotProduct(velDiff, normal) + 0.4f * (minDist + 1) / mFrameTime};

				if(remove < 0 && minDist < 0)
				{
					Vec2f impulse{normal * remove / (mBody.getInvMass() + b->getInvMass())};

					mBody.applyImpulse(impulse * mBody.getInvMass());
					b->applyImpulse(-impulse * b->getInvMass());
				 }

				if(std::abs(mBody.getVelocity().x) < std::abs(desiredX)) mBody.setVelocityX(desiredX * ssvu::getSign(mBody.getVelocity().x));
				if(std::abs(mBody.getVelocity().y) < std::abs(desiredY)) mBody.setVelocityY(desiredY * ssvu::getSign(mBody.getVelocity().y));
			}
		}
	};
}

#endif
