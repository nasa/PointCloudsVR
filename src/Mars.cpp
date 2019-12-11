#include "Mars.hpp"

Mars::Mars(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice)
	: OpenFrames::Sphere("Mars", 0, 1, 0, 0.9)
{
	/*osg::ref_ptr<osg::Program> program = new osg::Program();
	osg::ref_ptr<osg::Shader> fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "../../shaders/MarsAtmosphere.frag");
	program->addShader(fragShader);

	osg::ref_ptr<osg::StateSet> state = _sphereSD->getOrCreateStateSet();
	state->setAttributeAndModes(program, osg::StateAttribute::ON);

	osg::ref_ptr<osg::Uniform> viewMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "vm");
	viewMatrix->setUpdateCallback(new Mars::ViewMatrixCallback(ovrDevice));
	state->addUniform(viewMatrix);*/
}