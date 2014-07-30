#pragma once
#include <RendererWindow.h>
#include <Renderer.h>

class Window_MD : public Engine::RendererWindow
{
public:
	Window_MD();
	uint oldestRippleIndex;
	uint oldestWaveIndex;
	uint framesSinceLastKey;
	Engine::Renderer::UniformInfo* meshFacing;
	Engine::Renderer::UniformBlockInfo* ripples;
	Engine::Renderer::UniformBlockInfo* waves;
	Engine::Renderer::UniformInfo* crosshairTransform;
	virtual void initialize();
	virtual void inheritedUpdate();
};

