#include "ViewController.h"
#include "../Log.h"
#include "../SystemData.h"

#include "BasicGameListView.h"
#include "DetailedGameListView.h"
#include "GridGameListView.h"
#include "../animations/LaunchAnimation.h"
#include "../animations/MoveCameraAnimation.h"

ViewController::ViewController(Window* window)
	: GuiComponent(window), mCurrentView(nullptr), mCamera(Eigen::Affine3f::Identity()), mFadeOpacity(0)
{
	mState.viewing = START_SCREEN;
}

void ViewController::goToSystemSelect()
{
	mState.viewing = SYSTEM_SELECT;
	goToSystem(SystemData::sSystemVector.at(0));
	//playViewTransition();
}

SystemData* getSystemCyclic(SystemData* from, bool reverse)
{
	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;

	if(reverse)
	{
		auto it = std::find(sysVec.rbegin(), sysVec.rend(), from);
		assert(it != sysVec.rend());
		it++;
		if(it == sysVec.rend())
			it = sysVec.rbegin();
		return *it;
	}else{
		auto it = std::find(sysVec.begin(), sysVec.end(), from);
		assert(it != sysVec.end());
		it++;
		if(it == sysVec.end())
			it = sysVec.begin();
		return *it;
	}
}

void ViewController::goToNextSystem()
{
	assert(mState.viewing == SYSTEM);

	SystemData* system = mState.data.system;
	if(system == NULL)
		return;
	
	goToSystem(getSystemCyclic(system, false));
}

void ViewController::goToPrevSystem()
{
	assert(mState.viewing == SYSTEM);

	SystemData* system = mState.data.system;
	if(system == NULL)
		return;
	
	goToSystem(getSystemCyclic(system, true));
}

void ViewController::goToSystem(SystemData* system)
{
	mState.viewing = SYSTEM;
	mState.data.system = system;

	mCurrentView = getSystemView(system);
	playViewTransition();
}

void ViewController::playViewTransition()
{
	setAnimation(new MoveCameraAnimation(mCamera, mCurrentView->getPosition()));
}

void ViewController::onFileChanged(FileData* file, FileChangeType change)
{
	for(auto it = mSystemViews.begin(); it != mSystemViews.end(); it++)
	{
		it->second->onFileChanged(file, change);
	}
}

void ViewController::launch(FileData* game, Eigen::Vector3f center)
{
	if(game->getType() != GAME)
	{
		LOG(LogError) << "tried to launch something that isn't a game";
		return;
	}

	
	game->getSystem()->getTheme()->playSound("gameSelectSound");

	Eigen::Affine3f origCamera = mCamera;
	center += mCurrentView->getPosition();
	setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 1500), [this, origCamera, center, game] 
	{
		game->getSystem()->launchGame(mWindow, game);
		mCamera = origCamera;
		setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 600), nullptr, true);
	});
}

std::shared_ptr<GameListView> ViewController::getSystemView(SystemData* system)
{
	//if we already made one, return that one
	auto exists = mSystemViews.find(system);
	if(exists != mSystemViews.end())
		return exists->second;

	//if we didn't, make it, remember it, and return it
	std::shared_ptr<GameListView> view;

	if(system != NULL)
	{
		//decide type
		bool detailed = false;
		std::vector<FileData*> files = system->getRootFolder()->getFilesRecursive(GAME | FOLDER);
		for(auto it = files.begin(); it != files.end(); it++)
		{
			if(!(*it)->getThumbnailPath().empty())
			{
				detailed = true;
				break;
			}
		}
		
		if(detailed)
			view = std::shared_ptr<GameListView>(new DetailedGameListView(mWindow, system->getRootFolder()));
		else
			view = std::shared_ptr<GameListView>(new BasicGameListView(mWindow, system->getRootFolder()));
		
		//view = std::shared_ptr<GameListView>(new GridGameListView(mWindow, system->getRootFolder()));

		view->setTheme(system->getTheme());
	}else{
		LOG(LogError) << "null system"; // should eventually return an "all games" gamelist view
	}

	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	int id = std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();
	view->setPosition(id * (float)Renderer::getScreenWidth(), 0);

	mSystemViews[system] = view;
	return view;
}



bool ViewController::input(InputConfig* config, Input input)
{
	if(mCurrentView)
		return mCurrentView->input(config, input);

	return false;
}

void ViewController::update(int deltaTime)
{
	if(mCurrentView)
	{
		mCurrentView->update(deltaTime);
	}

	GuiComponent::update(deltaTime);
}

void ViewController::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = mCamera * parentTrans;

	// camera position, position + size
	Eigen::Vector3f viewStart = trans.inverse().translation();
	Eigen::Vector3f viewEnd = trans.inverse() * Eigen::Vector3f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight(), 0);

	// draw systems
	for(auto it = mSystemViews.begin(); it != mSystemViews.end(); it++)
	{
		// clipping
		Eigen::Vector3f guiStart = it->second->getPosition();
		Eigen::Vector3f guiEnd = it->second->getPosition() + Eigen::Vector3f(it->second->getSize().x(), it->second->getSize().y(), 0);

		if(guiEnd.x() >= viewStart.x() && guiEnd.y() >= viewStart.y() &&
			guiStart.x() <= viewEnd.x() && guiStart.y() <= viewEnd.y())
				it->second->render(trans);
	}

	// fade out
	if(mFadeOpacity)
	{
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x00000000 | (unsigned char)(mFadeOpacity * 255));
	}
}
