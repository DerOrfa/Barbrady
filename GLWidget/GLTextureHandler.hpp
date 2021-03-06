#ifndef GLMEMORYMANAGER_HPP
#define GLMEMORYMANAGER_HPP

#include "GLCommon.hpp"
#include "datacontainer.hpp"
#include "viewercorebase.hpp"
#include <DataStorage/typeptr.hpp>
#include <DataStorage/image.hpp>

#include <limits>

namespace isis
{
namespace viewer
{
namespace GL
{

/**
 * This class is responsible for copying the image data to a GL_TEXTURE_3D.
 * It gets the data as a boost::weak_ptr and copies it to the texture.
 */

class GLTextureHandler
{
public:

	enum InterpolationType { neares_neighbor, linear };

	GLTextureHandler() { m_Alpha = false; }

	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	typedef std::map<boost::shared_ptr<ImageHolder>, std::map<size_t, GLuint > > ImageMapType;

	///Convinient function to copy all in DataContainer available volumes to a GL_TEXTURE_3D.
	std::map<boost::shared_ptr<ImageHolder>, GLuint> copyAllImagesToTextures( const DataContainer &data, const bool withAlpha = true, InterpolationType interpolation = neares_neighbor );

	///Copies the given timestep of an image with the given imageID to a GL_TEXTURE_3D. Return the texture id.
	GLuint copyImageToTexture( const boost::shared_ptr<ImageHolder> image, size_t timestep, const bool withAlpha = true, InterpolationType interpolation = neares_neighbor );

	///The image map is a mapping of the imageID and timestep to the texture of the GL_TEXTURE_3D.
	ImageMapType getImageMap() const { return m_ImageMap; }

	bool forceReloadingAllOfType( ImageHolder::ImageType imageType, InterpolationType interpolation = neares_neighbor, bool withAlpha = true ) ;

	void setAlphaEnabled( bool enabled ) { m_Alpha = enabled; }

private:
	ImageMapType m_ImageMap;
	//this is only needed if one specifies the manual scaling
	bool m_Alpha;

	template<typename TYPE>
	GLuint internCopyImageToTexture( GLenum format, const boost::shared_ptr<ImageHolder> image, size_t timestep, bool alpha = true, InterpolationType interpolation = neares_neighbor  ) {
		LOG( Debug, info ) << "Copy image " << image->getID() << " with timestep " << timestep << " to texture";

		util::FixedVector<size_t, 4> size = image->getImageSize();
		size_t volume = size[0] * size[1] * size[2];
		TYPE *dataPtr = static_cast<TYPE *>( image->getImageWeakPointer( timestep ).lock().get() );
		assert( dataPtr != 0 );
		GLint interpolationType = 0;

		switch ( interpolation ) {
		case neares_neighbor:
			interpolationType = GL_NEAREST;
			break;
		case linear:
			interpolationType = GL_LINEAR;
			break;
		}

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if( !gluCheckExtension( ( const GLubyte * )"GL_ARB_texture_non_power_of_two", glGetString( GL_EXTENSIONS ) ) ) {
			LOG( Runtime, error ) << "Your OpenGL version does not support image sizes unequal n^2. The viewer is not yet capable of working with such OpenGL versions.";
			return 0;
		}

		//check if max texture size is bigger than image size
		GLint texSize;
		glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &texSize );
		bool pass = true;

		for( size_t i = 0; i < 3; i++ ) {
			if( size[i] > static_cast<GLuint>( texSize ) ) {
				pass = false;
			}
		}

		if( !pass ) {
			LOG( Runtime, error ) << "Your image size " << size
								  << " exceeds the maximum texture size of your OpenGL implementation "
								  << texSize << ". Can not load image " << image->getID() << " ("
								  << image->getFileNames().front() << ") !";
			return 0;
		}

		//look if this texture already exists
		GLuint texture;

		if( m_ImageMap[image].find( timestep ) == m_ImageMap[image].end() ) {
			glGenTextures( 1, &texture );
		} else {
			texture = m_ImageMap[image].at( timestep );
		}

		glBindTexture( GL_TEXTURE_3D, texture );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, interpolationType );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
		GLint internalFormat;
		GLenum dataFormat;

		if( alpha ) {
			TYPE *dataWithAplpha = ( TYPE * ) calloc( volume * 2, sizeof( TYPE ) );
			size_t index = 0;

			for ( size_t i = 0; i < volume * 2; i += 2 ) {
				dataWithAplpha[i] = dataPtr[index++];
				dataWithAplpha[i + 1] = std::numeric_limits<TYPE>::max();
			}

			glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE12_ALPHA4,
						  size[0],
						  size[1],
						  size[2], 0, GL_LUMINANCE_ALPHA, format,
						  dataWithAplpha );
		} else {
			glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE,
						  size[0],
						  size[1],
						  size[2], 0, GL_LUMINANCE, format,
						  dataPtr );
		}

		std::stringstream context;
		context << "loading timestep " << timestep << " of image " << image->getID() << " to glTexture3D";

		if( checkAndReportGLError( context.str() ) ) {
			return 0;
		} else {
			m_ImageMap[image].insert( std::make_pair<size_t, GLuint >( timestep, texture ) );
			return texture;
		}
	}


};

}
}
}

#endif