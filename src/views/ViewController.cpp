#include "ViewController.h"
#include "../Log.h"
#include "../SystemData.h"
#include "../Settings.h"

#include "gamelist/BasicGameListView.h"
#include "gamelist/DetailedGameListView.h"
#include "gamelist/GridGameListView.h"
#include "../guis/GuiMenu.h"
#include "../animations/LaunchAnimation.h"
#include "../animations/MoveCameraAnimation.h"
#include "../animations/LambdaAnimation.h"

ViewController::ViewController(Window* window)
	: GuiComponent(window), mCurrentView(nullptr), mCamera(Eigen::Affine3f::Identity()), mFadeOpacity(0), mLockInput(false)
{
	mState.viewing = NOTHING;
}

void ViewController::goToStart()
{
	// TODO
	/* mState.viewing = START_SCREEN;
	mCurrentView.reset();
	playViewTransition(); */
	goToSystemView(SystemData::sSystemVector.at(0));
}

void ViewController::goToSystemView(SystemData* system)
{
	mState.viewing = SYSTEM_SELECT;
	mState.system = system;

	getSystemListView()->goToSystem(system);
	mCurrentView = getSystemListView();
	updateHelpPrompts();
	playViewTransition();
}

void ViewController::goToNextGameList()
{
	assert(mState.viewing == GAME_LIST);
	SystemData* system = getState().getSystem();
	assert(system);
	goToGameList(system->getNext());
}

void ViewController::goToPrevGameList()
{
	assert(mState.viewing == GAME_LIST);
	SystemData* system = getState().getSystem();
	assert(system);
	goToGameList(system->getPrev());
}

void ViewController::goToGameList(SystemData* system)
{
	mState.viewing = GAME_LIST;
	mState.system = system;

	mCurrentView = getGameListView(system);
	updateHelpPrompts();
	playViewTransition();
}

void ViewController::playViewTransition()
{
	Eigen::Vector3f target(Eigen::Vector3f::Identity());
	if(mCurrentView) 
		target = mCurrentView->getPosition();

	if(Settings::getInstance()->getString("TransitionStyle") == "fade")
	{
		// fade animation
		auto fadeAnim = [this, target](float t) {
			float fadeStart = lerp<float>(0, 1, t / 0.3f);
			float fadeEnd = lerp<float>(1, 0, (t - 0.7f) / 0.3f);

			if(t <= 0.3f)
			{
				mFadeOpacity = fadeStart;
			}else{
				this->mCamera.translation() = -target;
				mFadeOpacity = fadeEnd;
			}
		};
		setAnimation(new LambdaAnimation(fadeAnim, 800));
	}else{
		// slide
		setAnimation(new MoveCameraAnimation(mCamera, target));
	}
}

void ViewController::onFileChanged(FileData* file, FileChangeType change)
{
	auto it = mGameListViews.find(file->getSystem());
	if(it != mGameListViews.end())
		it->second->onFileChanged(file, change);
}

void ViewController::launch(FileData* game, Eigen::Vector3f center)
{
	if(game->getType() != GAME)
	{
		LOG(LogError) << "tried to launch something that isn't a game";
		return;
	}

	Eigen::Affine3f origCamera = mCamera;
	origCamera.translation() = -mCurrentView->getPosition();

	center += mCurrentView->getPosition();
	stopAnimation(1); // make sure the fade in isn't still playing
	mLockInput = true;
	setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 1500), [this, origCamera, center, game] 
	{
		game->getSystem()->launchGame(mWindow, game);
		mCamera = origCamera;
		mLockInput = false;
		setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 600), nullptr, true);
		this->onFileChanged(game, FILE_METADATA_CHANGED);
	});
}

std::shared_ptr<IGameListView> ViewController::getGameListView(SystemData* system)
{
	//if we already made one, return that one
	auto exists = mGameListViews.find(system);
	if(exists != mGameListViews.end())
		return exists->second;

	//if we didn't, make it, remember it, and return it
	std::shared_ptr<IGameListView> view;

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
		view = std::shared_ptr<IGameListView>(new DetailedGameListView(mWindow, system->getRootFolder()));
	else
		view = std::shared_ptr<IGameListView>(new BasicGameListView(mWindow, system->getRootFolder()));
		
	// uncomment for experimental "image grid" view
	//view = std::shared_ptr<IGameListView>(new GridGameListView(mWindow, system->getRootFolder()));

	view->setTheme(system->getTheme());

	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	int id = std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();
	view->setPosition(id * (float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight() * 2);

	mGameListViews[system] = view;
	return view;
}

std::shared_ptr<SystemView> ViewController::getSystemListView()
{
	//if we already made one, return that one
	if(mSystemListView)
		return mSystemListView;

	mSystemListView = std::shared_ptr<SystemView>(new SystemView(mWindow));
	mSystemListView->setPosition(0, (float)Renderer::getScreenHeight());
	return mSystemListView;
}


bool ViewController::input(InputConfig* config, Input input)
{
	if(mLockInput)
		return true;

	// open menu
	if(config->isMappedTo("start", input) && input.value != 0)
	{
		// open menu
		mWindow->pushGui(new GuiMenu(mWindow));
		return true;
	}

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

	// draw systemview
	getSystemListView()->render(trans);
	
	// draw gamelists
	for(auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
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
		Renderer::setMatrix(parentTrans);
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x00000000 | (unsigned char)(mFadeOpacity * 255));
	}
}

void ViewController::preload()
{
	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++)
	{
		getGameListView(*it);
	}
}

void ViewController::reloadGameListView(IGameListView* view, bool reloadTheme)
{
	for(auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		if(it->second.get() == view)
		{
			bool isCurrent = (mCurrentView == it->second);
			SystemData* system = it->first;
			FileData* cursor = view->getCursor();
			mGameListViews.erase(it);

			if(reloadTheme)
				system->loadTheme();

			std::shared_ptr<IGameListView> newView = getGameListView(system);
			newView->setCursor(cursor);

			if(isCurrent)
				mCurrentView = newView;

			break;
		}
	}
}

std::vector<HelpPrompt> ViewController::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	if(!mCurrentView)
		return prompts;
	
	prompts = mCurrentView->getHelpPrompts();
	prompts.push_back(HelpPrompt("start", "menu"));

	return prompts;
}
