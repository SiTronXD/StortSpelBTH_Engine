#include "PathFinding.h"

PathFindingManager::PathFindingManager() {}

glm::vec3 PathFindingManager::getDirTo(glm::vec3& from, glm::vec3& to)
{
  if (polygons.size() != pf.getSizeOfPolygons())
    {
      //hopefully this won't be called to often
      pf.AddPolygons(polygons, 5);
      std::cout << "changed map" << std::endl;
      //change -1 based on how big our enemies are
    }
  std::vector<NavMesh::Point> externalPoints;
  NavMesh::Point start(fromVec3ToPoint(from));
  NavMesh::Point goal(fromVec3ToPoint(to));
  externalPoints.push_back(start);
  externalPoints.push_back(goal);

  pf.AddExternalPoints(externalPoints);

  std::vector<NavMesh::Point> pointsPath = pf.GetPath(start, goal);

  if (pointsPath.size() > 1)
    {
      NavMesh::Point dir = start - pointsPath[1];
      //0 is the point where we already are at
      return -normalize(dir.fromPointsToVec3());  
    }
  return glm::vec3(0, 0, 0);  //we shall stand still
}

void PathFindingManager::addPolygon(NavMesh::Polygon polygon)
{
  polygons.push_back(polygon);
}

void PathFindingManager::removePolygon(int id)
{
  polygons.erase(polygons.begin() + id);
}

void PathFindingManager::removeAllPolygons()
{
  polygons.clear();
}