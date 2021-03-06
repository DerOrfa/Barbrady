#include "GLShaderHandler.hpp"
#include <boost/foreach.hpp>


namespace isis
{
namespace viewer
{
namespace GL
{

GLShaderHandler::GLShaderHandler()
	: m_isEnabled( false ),
	  m_Context( false )
{

}

void GLShaderHandler::addShader( const std::string &name, const std::string &source, const isis::viewer::GL::GLShader::ShaderType &shaderType )
{
	GLShader shader;
	shader.setShaderType( shaderType );
	shader.setSourceCode( source );

	switch( shaderType ) {
	case GLShader::fragment:
		shader.setShaderID( glCreateShader( GL_FRAGMENT_SHADER ) );
		break;
	case GLShader::vertex:
		shader.setShaderID( glCreateShader( GL_VERTEX_SHADER ) );
		break;
	}

	const GLchar *csource  = source.c_str();

	glShaderSource( shader.getShaderID(), 1, &csource, NULL );

	//  glShaderSource( shader.getShaderID(), 1, &csource, NULL );

	glCompileShader( shader.getShaderID() );

	GLint compileStatus;

#ifndef __APPLE__
	glGetShaderiv( static_cast<GLuint>( shader.getShaderID() ), GL_COMPILE_STATUS, &compileStatus );

	if( compileStatus == GL_FALSE ) {
		LOG( Runtime, error ) << "Error during compilation of shader " << name << " !";
	} else {
		LOG( Debug, info ) << "Shader " << name << " with id " << shader.getShaderID() << " compiled successfully!";
	}

#else
	checkAndReportGLError( "compiling shader" + name );
#endif
	m_ShaderMap[name] = shader;
}

void GLShaderHandler::setEnabled( bool enable )
{
	if( !m_Context ) {
		LOG( Runtime, error ) << "You first have to create the shader context";
	}

	if( enable && !m_isEnabled ) {
		BOOST_FOREACH( ShaderMapType::const_reference shader, m_ShaderMap ) {
			glAttachShader( m_ProgramID, shader.second.getShaderID() );
			checkAndReportGLError( "attaching shader"  );
		}
		glLinkProgram( m_ProgramID );
		checkAndReportGLError( "glLinkProgramARB" );
		glValidateProgram( m_ProgramID );
		checkAndReportGLError( "glValidateProgramARB" );
		glUseProgram( m_ProgramID );
		m_isEnabled = true;
		checkAndReportGLError( "glUseProgramObjectARB" );
	} else if ( !enable && m_isEnabled ) {
		BOOST_FOREACH( ShaderMapType::const_reference shader, m_ShaderMap ) {
			glDetachShader( m_ProgramID, shader.second.getShaderID() );
			checkAndReportGLError( "detaching shader" );
		}
		m_isEnabled = false;
	}

}

void GLShaderHandler::removeShader( const std::string &name )
{
	glDetachShader( m_ProgramID, m_ShaderMap[name].getShaderID() );
	m_ShaderMap.erase( name );
}


void GLShaderHandler::addShader( const std::string &name, const isis::viewer::GL::GLShader &shader )
{
	m_ShaderMap[name] = shader;
}

}
}
} //end namespace