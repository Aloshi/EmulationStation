#include "SystemListView.h"
#include "../Renderer.h"
#include "../SystemData.h"
#include "../animations/MoveCameraAnimation.h"
#include "../Window.h"
#include "ViewController.h"

SystemListView::SystemListView(Window* window) : GuiComponent(window), mCamera(Eigen::Affine3f::Identity()), mCurrentSystem(NULL)
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	goToSystem(SystemData::sSystemVector.at(0));
}

void SystemListView::goToSystem(SystemData* system)
{
	std::shared_ptr<SystemView> view = getSystemView(system);
	mCurrentView = view;
	mCurrentSystem = system;
	setAnimation(new MoveCameraAnimation(mCamera, view->getPosition()));
}


std::shared_ptr<SystemView> SystemListView::getSystemView(SystemData* system)
{
	auto exists = mSystemViews.find(system);
	if(exists != mSystemViews.end())
		return exists->second;

	const std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	int id = std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();

	std::shared_ptr<SystemView> view = std::shared_ptr<SystemView>(new SystemView(mWindow, system));
	view->setPosition(id * (float)Renderer::getScreenWidth(), 0);

	mSystemViews[system] = view;

	return view;
}

bool SystemListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		int id = std::find(SystemData::sSystemVector.begin(), SystemData::sSystemVector.end(), mCurrentSystem) - SystemData::sSystemVector.begin();
		if(config->isMappedTo("left", input))
		{
			id--;
			if(id < 0)
				id += SystemData::sSystemVector.size();
			goToSystem(SystemData::sSystemVector.at(id));
			return true;
		}
		if(config->isMappedTo("right", input))
		{
			id = (id + 1) % SystemData::sSystemVector.size();
			goToSystem(SystemData::sSystemVector.at(id));
			return true;
		}
		if(config->isMappedTo("a", input))
		{
			mWindow->getViewController()->goToGameList(mCurrentSystem);
			return true;
		}
	}

	return GuiComponent::input(config, input);
}

void SystemListView::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = mCamera * getTransform() * parentTrans;

	// TODO: clipping
	for(auto it = mSystemViews.begin(); it != mSystemViews.end(); it++)
	{
		it->second->render(trans);
	}
}
