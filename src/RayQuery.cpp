#include "RayQuery.h"

#include "Ogre3d/include/OgreEntity.h"
#include "Ogre3d/include/OgreSubEntity.h"
#include "Ogre3d/include/OgreMesh.h"
#include "Ogre3d/include/OgreSubMesh.h"


// Get the mesh information for the given mesh.
// Code found in Wiki: www.ogre3d.org/wiki/index.php/RetrieveVertexData
void GetMeshInformation(const Ogre::MeshPtr mesh,
                                size_t &vertex_count,
                                Vector* &vertices,
                                size_t &index_count,
                                unsigned long* &indices,
                                const Vector &position,
                                const Quaternion &orient,
                                const Vector &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    vertex_count = index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }


    // Allocate space for the vertices and indices
    vertices = new Vector[vertex_count];
    indices = new unsigned long[index_count];

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Vector pt(pReal[0], pReal[1], pReal[2]);

                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }


        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                    static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
} 

void GetMeshInformation( Ogre::Entity *entity,
                                size_t &vertex_count,
                                Vector* &vertices,
                                size_t &index_count,
                                unsigned long* &indices,
                                const Vector &position,
                                const Quaternion &orient,
                                const Vector &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;
    vertex_count = index_count = 0;

   Ogre::MeshPtr mesh = entity->getMesh();


   bool useSoftwareBlendingVertices = entity->hasSkeleton();

   if (useSoftwareBlendingVertices)
   {
      entity->_updateAnimation();
   }

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }


    // Allocate space for the vertices and indices
    vertices = new Vector[vertex_count];
    indices = new unsigned long[index_count];

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

      //----------------------------------------------------------------
      // GET VERTEXDATA
      //----------------------------------------------------------------

        //Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
      Ogre::VertexData* vertex_data;

      //When there is animation:
      if(useSoftwareBlendingVertices)
#ifdef BUILD_AGAINST_AZATHOTH
         vertex_data = submesh->useSharedVertices ? entity->_getSharedBlendedVertexData() : entity->getSubEntity(i)->_getBlendedVertexData();
#else
         vertex_data = submesh->useSharedVertices ? entity->_getSkelAnimVertexData() : entity->getSubEntity(i)->_getSkelAnimVertexData();
#endif
      else
         vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;


        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Vector pt(pReal[0], pReal[1], pReal[2]);

                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }


        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                    static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
} 



void RayQuery::init()
{
	// create the ray scene query object
    m_pray_scene_query = ogreSceneManager->createRayQuery(Ogre::Ray(), Ogre::SceneManager::WORLD_GEOMETRY_TYPE_MASK);
    if (NULL == m_pray_scene_query)
        LOG_ERROR( "Failed to create Ogre::RaySceneQuery instance" );

	m_pray_scene_query->setSortByDistance(true); 

}

void RayQuery::end()
{
	//if ( ogreSceneManager )
	//	ogreSceneManager->destroyQuery( m_pray_scene_query );
}


// raycast from a point in to the scene.
// returns success or failure.
// on success the point is returned in the result.
Ogre::Entity* RayQuery::execute( Ogre::Camera* camera /*const Vector &point, const Vector &normal, Vector &result*/)
{
    // create the ray to test
    //Ogre::Ray ray(Vector(point.x, point.y, point.z),
    //              Vector(normal.x, normal.y, normal.z));

	// Julio: create ray from mouse
	Vector result;
	Ogre::Ray ray = camera->getCameraToViewportRay( mouseX/(float)width, mouseY/(float)height);
	
	// check we are initialised
    if (m_pray_scene_query != NULL)
    {
        // create a query object
        m_pray_scene_query->setRay(ray);

        // execute the query, returns a vector of hits
        if (m_pray_scene_query->execute().size() <= 0)
        {
            // raycast did not hit an objects bounding box
            return NULL;
        }
    }
    else
    {
        LOG_ERROR( "Cannot raycast without RaySceneQuery instance");
        return NULL;
    }   

	Ogre::Entity* hitEntity = NULL;

    // at this point we have raycast to a series of different objects bounding boxes.
    // we need to test these different objects to see which is the first polygon hit.
    // there are some minor optimizations (distance based) that mean we wont have to
    // check all of the objects most of the time, but the worst case scenario is that
    // we need to test every triangle of every object.
    Ogre::Real closest_distance = -1.0f;
    Vector closest_result;
    Ogre::RaySceneQueryResult &query_result = m_pray_scene_query->getLastResults();
    for (size_t qr_idx = 0; qr_idx < query_result.size(); qr_idx++)
    {
        // stop checking if we have found a raycast hit that is closer
        // than all remaining entities
        if ((closest_distance >= 0.0f) &&
            (closest_distance < query_result[qr_idx].distance))
        {
             break;
        }
       
        // only check this result if its a hit against an entity
        if ((query_result[qr_idx].movable != NULL) &&
            (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
        {
            // get the entity to check
            Ogre::Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable);           

            // mesh data to retrieve         
            size_t vertex_count;
            size_t index_count;
            Vector *vertices;
            unsigned long *indices;

            // get the mesh information
			GetMeshInformation(	pentity, vertex_count, vertices, index_count, indices,             
								pentity->getParentNode()->_getDerivedPosition(),
								pentity->getParentNode()->_getDerivedOrientation(),
								pentity->getParentNode()->_getDerivedScale());

            // test for hitting individual triangles on the mesh
            bool new_closest_found = false;
            for (int i = 0; i < static_cast<int>(index_count); i += 3)
            {
                // check for a hit against this triangle
                std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
                    vertices[indices[i+1]], vertices[indices[i+2]], true, false);

                // if it was a hit check if its the closest
                if (hit.first)
                {
                    if ((closest_distance < 0.0f) ||
                        (hit.second < closest_distance))
                    {
                        // this is the closest so far, save it off
                        closest_distance = hit.second;
                        new_closest_found = true;
                    }
                }
            }

         // free the verticies and indicies memory
            delete[] vertices;
            delete[] indices;

            // if we found a new closest raycast for this object, update the
            // closest_result before moving on to the next object.
            if (new_closest_found)
            {
                closest_result = ray.getPoint(closest_distance);   
				hitEntity = pentity;
            }
        }       
    }

    // return the result
    if (closest_distance >= 0.0f)
    {
        // raycast success
        result = closest_result;
        return hitEntity;
    }
    else
    {
        // raycast failed
        return NULL;
    } 
}
