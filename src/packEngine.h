
#pragma once

#include "cxy.h"

namespace raven
{
    namespace pack
    {
        class cItem
        {
        public:
            cxyz wlh;
            cxyz loc;

            cItem(double x, double y, double z)
                : wlh(x, y, z)
            {
            }
            cItem(double x, double y)
                : cItem(x, y, -DBL_MAX)
            {
            }
            void move(int x, int y, int z)
            {
                loc = cxyz(x, y, z);
            }
            void move(int x, int y)
            {
                loc = cxyz(x, y, -DBL_MAX);
            }
            void move(const cItem &other)
            {
                loc = other.loc;
            }
            double volume() const
            {
                if (abs(wlh.z) > 0.0001)
                    return abs(wlh.x * wlh.y * wlh.z);
                else
                    return abs(wlh.x * wlh.y);
            }
        };

        class cEngine
        {
        public:
            enum class eBestSpaceAlgo
            {
                firstFit,
                minGap,
                minDist,
            };
            void setBestSpaceAlgo(eBestSpaceAlgo algo)
            {
                myBestSpaceAlgo = algo;
            }
            void setSize(int x, int y)
            {
                f2d = true;
                mySpaces.clear();
                mySpaces.emplace_back(x, y, -DBL_MAX);
                mySpaces[0].move(0, 0);
            }
            void addItem(int x, int y, int z)
            {
                myItems.emplace_back(x, y, z);
            }
            void pack()
            {
                sort();

                for (auto &item : myItems)
                    pack(item);
            }
            void pack(cItem &item)
            {
                // find index of space where the box will fit
                int space = findBestSpace(item);
                if (space < 0)
                    throw std::runtime_error(
                        "No space for item");

                // move the box into the space
                item.move(mySpaces[space]);

                // // split the space into two smaller spaces
                // // one to the right, one below
                splitSpace(space, item);
            }

        private:
            bool f2d;
            eBestSpaceAlgo myBestSpaceAlgo;
            std::vector<cItem> myItems;
            std::vector<cItem> mySpaces;

            /* Sort boxes into order of decreasing volume

            This permits the smaller boxes to be packed into
            the spaces left behind by the larger boxes previously packed.

            */
            void sort()
            {
                std::sort(
                    myItems.begin(), myItems.end(),
                    [](const cItem &a, const cItem &b)
                    {
                        return a.volume() < b.volume();
                    });
            }
            int findBestSpace(const cItem &item)
            {
                int bestSpaceIndex = -1;
                double leastWastage = INT_MAX;
                double leastDistance = INT_MAX;
                for (int s = 0; s < mySpaces.size(); s++)
                {
                    // check for remains of a split space
                    if (mySpaces[s].loc.x < 0)
                        continue;
                    // check that space is tall enough for box
                    if (mySpaces[s].wlh.y < item.wlh.y)
                        continue;

                    // the box could be fitted into this space
                    // apply specified best space algorithm

                    switch (myBestSpaceAlgo)
                    {

                    case eBestSpaceAlgo::firstFit:
                        return s;

                    case eBestSpaceAlgo::minGap:
                    {
                        double wastage = mySpaces[s].wlh.y - item.wlh.y;
                        if (wastage < leastWastage)
                        {
                            leastWastage = wastage;
                            bestSpaceIndex = s;
                        }
                    }
                    break;

                    case eBestSpaceAlgo::minDist:
                    {
                        double distance = mySpaces[s].loc.x + mySpaces[s].loc.y;
                        if (distance < leastDistance)
                        {
                            leastDistance = distance;
                            bestSpaceIndex = s;
                        }
                    }
                    break;
                    }
                }
                return bestSpaceIndex;
            }
            void splitSpace(
                int ispace,
                const cItem &item)
            {
                if (!f2d)
                    throw std::runtime_error(
                        "splitSpace 1D and 3D NYI");
                cItem &sp0 = mySpaces[ispace];
                cItem sp1(sp0.wlh.x - item.wlh.x, item.wlh.y);
                sp1.move(sp0.loc.x + item.wlh.x, sp0.loc.y);
                cItem sp2(sp0.wlh.x, sp0.wlh.y - item.wlh.y);
                sp2.move(sp0.loc.x, sp0.loc.y + item.wlh.y);
                mySpaces.push_back(sp1);
                mySpaces.push_back(sp2);
                mySpaces[ispace].loc.x = -DBL_MAX;
            }
        };
    }
}