#ifndef RAY_QUERY_H
#define RAY_QUERY_H


#include "Cing.h"


class RayQuery
{
public:
	RayQuery(): m_pray_scene_query(0) { }
	~RayQuery() { end(); }

	void			init	();
	void			end		();
	Ogre::Entity*	execute	( Ogre::Camera* camera /*const Vector &point, const Vector &normal, Vector &result*/);

private:
	Ogre::RaySceneQuery*	m_pray_scene_query;

};

#endif