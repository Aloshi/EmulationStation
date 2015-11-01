#include <RecalboxConf.h>
#include <RecalboxSystem.h>
#include "views/ViewController.h"
#include "Log.h"
#include "SystemData.h"
#include "Settings.h"

#include "views/gamelist/BasicGameListView.h"
#include "views/gamelist/DetailedGameListView.h"
#include "views/gamelist/GridGameListView.h"
#include "guis/GuiMenu.h"
#include "guis/GuiMsgBox.h"
#include "animations/LaunchAnimation.h"
#include "animations/MoveCameraAnimation.h"
#include "animations/LambdaAnimation.h"

#include "AudioManager.h"


ViewController* ViewController::sInstance = NULL;

ViewController* ViewController::get()
{
	assert(sInstance);
	return sInstance;
}

void ViewController::init(Window* window)
{
	assert(!sInstance);
	sInstance = new ViewController(window);
}

ViewController::ViewController(Window* window)
	: GuiComponent(window), mCurrentView(nullptr), mCamera(Eigen::Affine3f::Identity()), mFadeOpacity(0), mLockInput(false)
{
	mState.viewing = NOTHING;
	mFavoritesOnly = Settings::getInstance()->getBool("FavoritesOnly");
}

ViewController::~ViewController()
{
	assert(sInstance == this);
	sInstance = NULL;
}

void ViewController::goToStart()
{
	// TODO
	/* mState.viewing = START_SCREEN;
	mCurrentView.reset();
	playViewTransition(); */
	goToSystemView(SystemData::sSystemVector.at(0));
}

int ViewController::getSystemId(SystemData* system)
{
	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	return std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();
}

void ViewController::goToSystemView(SystemData* system)
{
	mState.viewing = SYSTEM_SELECT;
	mState.system = system;

	auto systemList = getSystemListView();
	systemList->setPosition(getSystemId(system) * (float)Renderer::getScreenWidth(), systemList->getPosition().y());
	systemList->goToSystem(system, false);
	mCurrentView = systemList;

	playViewTransition();
}

void ViewController::goToNextGameList()
{
	assert(mState.viewing == GAME_LIST);
	SystemData* system = getState().getSystem();
	assert(system);
        AudioManager::getInstance()->startMusic(system->getNext()->getTheme());

	goToGameList(system->getNext());
}

void ViewController::goToPrevGameList()
{
	assert(mState.viewing == GAME_LIST);
	SystemData* system = getState().getSystem();
	assert(system);
        AudioManager::getInstance()->startMusic(system->getPrev()->getTheme());

	goToGameList(system->getPrev());
}

void ViewController::goToGameList(SystemData* system)
{
	if(mState.viewing == SYSTEM_SELECT)
	{
		// move system list
		auto sysList = getSystemListView();
		float offX = sysList->getPosition().x();
		int sysId = getSystemId(system);
		sysList->setPosition(sysId * (float)Renderer::getScreenWidth(), sysList->getPosition().y());
		offX = sysList->getPosition().x() - offX;
		mCamera.translation().x() -= offX;
	}

	if (mInvalidGameList[system] == true)
	{
		updateFavorite(system, getGameListView(system).get()->getCursor());
		if (mFavoritesOnly != Settings::getInstance()->getBool("FavoritesOnly"))
		{
			reloadGameListView(system);
			mFavoritesOnly = Settings::getInstance()->getBool("FavoritesOnly");
		}
		mInvalidGameList[system] = false;
	}

	mState.viewing = GAME_LIST;
	mState.system = system;

	mCurrentView = getGameListView(system);
	playViewTransition();
}

void ViewController::updateFavorite(SystemData* system, FileData* file)
{
	IGameListView* view = getGameListView(system).get();
	if (Settings::getInstance()->getBool("FavoritesOnly"))
	{
		const std::vector<FileData*>& files = system->getRootFolder()->getChildren();
		view->populateList(files);
		int pos = std::find(files.begin(), files.end(), file) - files.begin();
		bool found = false;
		for (auto it = files.begin() + pos; it != files.end(); it++)
		{
			if ((*it)->getType() == GAME)
			{
				if ((*it)->metadata.get("favorite").compare("yes") == 0)
				{
					view->setCursor(*it);
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			for (auto it = files.begin() + pos; it != files.begin(); it--)
			{
				if ((*it)->getType() == GAME)
				{
					if ((*it)->metadata.get("favorite").compare("yes") == 0)
					{
						view->setCursor(*it);
						break;
					}
				}
			}
		}

		if (!found)
		{
			view->setCursor(*(files.begin() + pos));
		}
	}

	view->updateInfoPanel();
}

void ViewController::playViewTransition()
{
	Eigen::Vector3f target(Eigen::Vector3f::Identity());
	if(mCurrentView)
		target = mCurrentView->getPosition();

	// no need to animate, we're not going anywhere (probably goToNextGamelist() or goToPrevGamelist() when there's only 1 system)
	if(target == -mCamera.translation() && !isAnimationPlaying(0))
		return;

	if(Settings::getInstance()->getString("TransitionStyle") == "fade")
	{
		// fade
		// stop whatever's currently playing, leaving mFadeOpacity wherever it is
		cancelAnimation(0);

		auto fadeFunc = [this](float t) {
			mFadeOpacity = lerp<float>(0, 1, t);
		};

		const static int FADE_DURATION = 240; // fade in/out time
		const static int FADE_WAIT = 320; // time to wait between in/out
		setAnimation(new LambdaAnimation(fadeFunc, FADE_DURATION), 0, [this, fadeFunc, target] {
			this->mCamera.translation() = -target;
			updateHelpPrompts();
			setAnimation(new LambdaAnimation(fadeFunc, FADE_DURATION), FADE_WAIT, nullptr, true);
		});

		// fast-forward animation if we're partway faded
		if(target == -mCamera.translation())
		{
			// not changing screens, so cancel the first half entirely
			advanceAnimation(0, FADE_DURATION);
			advanceAnimation(0, FADE_WAIT);
			advanceAnimation(0, FADE_DURATION - (int)(mFadeOpacity * FADE_DURATION));
		}else{
			advanceAnimation(0, (int)(mFadeOpacity * FADE_DURATION));
		}
	}else{
		// slide
		setAnimation(new MoveCameraAnimation(mCamera, target));
		updateHelpPrompts(); // update help prompts immediately
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

	if(Settings::getInstance()->getString("TransitionStyle") == "fade")
	{
		// fade out, launch game, fade back in
		auto fadeFunc = [this](float t) {
			//t -= 1;
			//mFadeOpacity = lerp<float>(0.0f, 1.0f, t*t*t + 1);
			mFadeOpacity = lerp<float>(0.0f, 1.0f, t);
		};
		setAnimation(new LambdaAnimation(fadeFunc, 800), 0, [this, game, fadeFunc]
		{
			game->getSystem()->launchGame(mWindow, game);
			mLockInput = false;
			setAnimation(new LambdaAnimation(fadeFunc, 800), 0, nullptr, true);
			this->onFileChanged(game, FILE_METADATA_CHANGED);
		});
	}else{
		// move camera to zoom in on center + fade out, launch game, come back in
		setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 1500), 0, [this, origCamera, center, game]
		{
			game->getSystem()->launchGame(mWindow, game);
			mCamera = origCamera;
			mLockInput = false;
			setAnimation(new LaunchAnimation(mCamera, mFadeOpacity, center, 600), 0, nullptr, true);
			this->onFileChanged(game, FILE_METADATA_CHANGED);
		});
	}
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
		view = std::shared_ptr<IGameListView>(new DetailedGameListView(mWindow, system->getRootFolder(), system));
	else
		view = std::shared_ptr<IGameListView>(new BasicGameListView(mWindow, system->getRootFolder()));

	// uncomment for experimental "image grid" view
	//view = std::shared_ptr<IGameListView>(new GridGameListView(mWindow, system->getRootFolder()));

	view->setTheme(system->getTheme());

	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	int id = std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();
	view->setPosition(id * (float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight() * 2);

	addChild(view.get());

	mGameListViews[system] = view;
	mInvalidGameList[system] = false;
	return view;
}

std::shared_ptr<SystemView> ViewController::getSystemListView()
{
	//if we already made one, return that one
	if(mSystemListView)
		return mSystemListView;

	mSystemListView = std::shared_ptr<SystemView>(new SystemView(mWindow));
	addChild(mSystemListView.get());
	mSystemListView->setPosition(0, (float)Renderer::getScreenHeight());
	return mSystemListView;
}


bool ViewController::input(InputConfig* config, Input input)
{
	if(mLockInput)
		return true;

	// open menu
	if(config->isMappedTo("start", input) && input.value != 0 && RecalboxConf::getInstance()->get("system.es.menu") != "none" )
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

	updateSelf(deltaTime);
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

	if(mWindow->peekGui() == this)
		mWindow->renderHelpPromptsEarly();

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

void ViewController::reloadAll()
{
	std::map<SystemData*, FileData*> cursorMap;
	for(auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		cursorMap[it->first] = it->second->getCursor();
	}
	mGameListViews.clear();

	for(auto it = cursorMap.begin(); it != cursorMap.end(); it++)
	{
		it->first->loadTheme();
		getGameListView(it->first)->setCursor(it->second);
	}

	mSystemListView.reset();
	getSystemListView();

	// update mCurrentView since the pointers changed
	if(mState.viewing == GAME_LIST)
	{
		mCurrentView = getGameListView(mState.getSystem());
	}else if(mState.viewing == SYSTEM_SELECT)
	{
		mSystemListView->goToSystem(mState.getSystem(), false);
		mCurrentView = mSystemListView;
	}else{
		goToSystemView(SystemData::sSystemVector.front());
	}

	updateHelpPrompts();
}

void ViewController::reloadGamesLists()
{
	mGameListViews.clear();

	if(mState.viewing == GAME_LIST)
	{
		mCurrentView = getGameListView(mState.getSystem());
	}else if(mState.viewing == SYSTEM_SELECT)
	{
		mSystemListView->goToSystem(mState.getSystem(), false);
		mCurrentView = mSystemListView;
	}else{
		goToSystemView(SystemData::sSystemVector.front());
	}
}

void ViewController::setInvalidGamesList(SystemData* system)
{
	for (auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		if (system == (it->first))
		{
			mInvalidGameList[it->first] = true;
			break;
		}
	}
}

void ViewController::setAllInvalidGamesList(SystemData* systemExclude)
{
	for (auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		if (systemExclude != (it->first))
		{
			mInvalidGameList[it->first] = true;
		}
	}
}

std::vector<HelpPrompt> ViewController::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	if(!mCurrentView)
		return prompts;

	prompts = mCurrentView->getHelpPrompts();
	if(RecalboxConf::getInstance()->get("system.es.menu") != "none"){
		prompts.push_back(HelpPrompt("start", "menu"));
	}

	return prompts;
}

HelpStyle ViewController::getHelpStyle()
{
	if(!mCurrentView)
		return GuiComponent::getHelpStyle();

	return mCurrentView->getHelpStyle();
}
