#include "views/ViewController.h"
#include "Log.h"
#include "SystemData.h"
#include "Settings.h"

#include "views/SystemView.h"
#include "views/gamelist/BasicGameListView.h"
#include "views/gamelist/DetailedGameListView.h"
#include "views/gamelist/VideoGameListView.h"
#include "views/gamelist/GridGameListView.h"
#include "guis/GuiMenu.h"
#include "guis/GuiMsgBox.h"
#include "animations/LaunchAnimation.h"
#include "animations/MoveCameraAnimation.h"
#include "animations/LambdaAnimation.h"
#include <SDL.h>

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
}

ViewController::~ViewController()
{
	assert(sInstance == this);
	sInstance = NULL;
}

void ViewController::goToStart()
{
	LOG(LogDebug) << "ViewController::goToStart()";
	goToSystemView(SystemData::sSystemVector.at(0));
}

int ViewController::getSystemId(SystemData* system)
{
	std::vector<SystemData*>& sysVec = SystemData::sSystemVector;
	return std::find(sysVec.begin(), sysVec.end(), system) - sysVec.begin();
}

void ViewController::goToSystemView(SystemData* system)
{
	// Tell any current view it's about to be hidden
	if (mCurrentView)
	{
		mCurrentView->onHide();
	}

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

	// skip systems that don't have a game list, this will always end since it is called
	// from a system with a game list and the iterator is cyclic
	do {
		system = system->getNext();
	} while ( system->getGameCount(true) == 0 );
	
	goToGameList(system);
}

void ViewController::goToPrevGameList()
{
	assert(mState.viewing == GAME_LIST);
	SystemData* system = getState().getSystem();
	assert(system);

	// skip systems that don't have a game list, this will always end since it is called
	// from a system with a game list and the iterator is cyclic
	do {
		system = system->getPrev();
	} while ( system->getGameCount(true) == 0 );
	
	goToGameList(system);
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
		reloadGameListView(system);
		mInvalidGameList[system] = false;
	}

	mState.viewing = GAME_LIST;
	mState.system = system;

	if (mCurrentView)
	{
		mCurrentView->onHide();
	}
	mCurrentView = getGameListView(system);
	if (mCurrentView)
	{
		mCurrentView->onShow();
	}
	playViewTransition();
}

/*
// Update the cursor location to the nearest valid location, before re-populating
// the gamelist to reflect new filter-values.
void ViewController::updateCursor(SystemData* system, FileData* file)
{
	LOG(LogDebug) << "ViewController::nudgeCursor()";
	// There are two scenario's here: 
	// 1) a single file's metadata has changed, toggling its
	// validity in the current view. In this case, some consistency in the cursor location is wanted.
	// 2) the view itself has changed, changing the visibility of many
	// items, so the cursor's consistency is not relevant.

	// get the old size of valid items
	IGameListView* view = getGameListView(system).get();
	int nr_old = view->getChildCount();
	
	// populate new view
	const std::vector<FileData*>& newfiles = system->getRootFolder()->getChildren(true);
	view->populateList(newfiles);
	
	// get pos in old list
	int pos = std::find(fullfiles.begin(), fullfiles.end(), file) - fullfiles.begin();					// save current position in that view
	// try to reproduce that pos in new list
	
	
	
	
	
	
	view->populateList(newfilesfiles);														// populate a new view 
	
	
	bool found = false;
	int count = files.size();
	for (int i = 0; i < count; i++) 
	{   
		// This does an inside-out search from pos, pos+1, pos-1, pos+2, pos-2 etc to find the closest match.
		int index = (pos + ((i%2==0) ? i/2 : count-(i+1)/2) ) % count;			
		if (files.at(index)->getType() == GAME)
		{
			view->setCursor(*it);														// and set the cursor to that one
			found = true;
			break;
		}
	}

	if (!found)
	{
		//If there is no hit in the current list, move to system selection view (aka start)
		goToStart();
		//view->setCursor(*(files.begin() + pos));
		
	}

	view->updateInfoPanel(); //update metadata display
	//view->populateList(files);	
	//view->setCursor(*(files.begin())); 
	//view->updateInfoPanel(); 
}
*/
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

// This function triggers the calls to onFileChanged in BasicGameListView and ISimpleGameListView
void ViewController::onFileChanged(FileData* file, FileChangeType change)
{
	LOG(LogDebug) << "ViewController::onFileChanged()";
	
	// check if the file is part of current collection of IGameListViews
	auto it = mGameListViews.find(file->getSystem());
	if(it != mGameListViews.end())
	{
		if((change == FILE_FILTERED) || (change == FILE_SORTED)) // affects all GameListViews
		{
			LOG(LogDebug) << "ViewController::onFileChanged(): change might affect all systems, setting all invalid";
			reloadSystemListView();
			setAllInvalidGamesList(file->getSystem());
		}
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

	// Hide the current view
	if (mCurrentView)
		mCurrentView->onHide();

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

// This function populates mGameListViews, which maps systemData to IGameListViews:
// like this: std::map< SystemData*, std::shared_ptr<IGameListView> >
std::shared_ptr<IGameListView> ViewController::getGameListView(SystemData* system, bool forceReload)
{
	//if we already made one, return that one
	auto exists = mGameListViews.find(system);
	if(exists != mGameListViews.end() && !forceReload)
		return exists->second;

	//if we didn't, or we want to reload anyway, make it, remember it, and return it
	std::shared_ptr<IGameListView> view;

	bool themeHasVideoView = system->getTheme()->hasView("video");

	//decide type
	bool detailed = false;
	bool video	  = false;
	std::vector<FileData*> files = system->getRootFolder()->getFilesRecursive(GAME | FOLDER);

	for(auto it = files.begin(); it != files.end(); it++)
	{
		if(themeHasVideoView && !(*it)->getVideoPath().empty())
		{
			video = true;
			break;
		}
		else if(!(*it)->getThumbnailPath().empty())
		{
			detailed = true;
			// Don't break out in case any subsequent files have video
		}
	}
	if (video)
		// Create the view
		view = std::shared_ptr<IGameListView>(new VideoGameListView(mWindow, system->getRootFolder()));
	else if(detailed)
		view = std::shared_ptr<IGameListView>(new DetailedGameListView(mWindow, system->getRootFolder(), system)); // TODO: remove this last argument!
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

// This function finds the current IGameListView, stores the cursor, loads
// a new gamelistview for system, and sets the cursor again.
// TODO: what if the cursor position (file) is no longer valid?
void ViewController::reloadGameListView(IGameListView* oldview, bool reloadTheme)
{
	LOG(LogDebug) << "ViewController::reloadGameListView()";
	for(auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		if(it->second.get() == oldview)
		{
			LOG(LogDebug) << "ViewController::reloadGameListView(): found GameListView for " << oldview->getName();

			bool isCurrent = (mCurrentView == it->second);
			SystemData* system = it->first;
			FileData* cursor = oldview->getCursor();
			mGameListViews.erase(it);

			if(reloadTheme)
				system->loadTheme();

			LOG(LogDebug) << "ViewController::reloadGameListView(): getting new GameListView for system: " << system->getName();

			std::shared_ptr<IGameListView> newView = getGameListView(system); // always returns a fresh one, as we just erased the stale one.
			
			LOG(LogDebug) << "ViewController::reloadGameListView(): List number of entries = "<< system->getGameCount(true);
			
			if (system->getGameCount(true) > 0)
			{
				newView->setCursor(cursor);

				if(isCurrent){
					mCurrentView = newView;
				}
			} else
			{
				LOG(LogDebug) << "ViewController::reloadGameListView(): No files left in view, go to start.";
				goToStart();
				break;
			}	
			//newView->updateInfoPanel();
				
			break;
		}
	}

	// Redisplay the current view
	if (mCurrentView)
		mCurrentView->onShow();
}

// This reloads all gameslists. This is an expensive operation!
// The UI remains much more response when a gameslist is only reloaded when
// its actually being requested by the user.
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
		SystemData* system = mState.getSystem();
		goToSystemView(SystemData::sSystemVector.front());
		mSystemListView->goToSystem(system, false);
		mCurrentView = mSystemListView;
	}else{
		goToSystemView(SystemData::sSystemVector.front());
	}

	updateHelpPrompts();
}

// This function sets a specific system's gameslist to be invalid,
// so it is reloaded when requested.
void ViewController::setInvalidGamesList(SystemData* system)
{
	LOG(LogDebug) << "ViewController::setInvalidGamesList()";
	for (auto it = mGameListViews.begin(); it != mGameListViews.end(); it++)
	{
		if (system == (it->first))
		{
			mInvalidGameList[it->first] = true;
			break;
		}
	}
}

// This function sets the gameslists for all systems (except for the systemExclude one)
// to invalid, this will cause the gamelist to be reloaded on demand.
// This is more efficient than calling reloadAll everytime.
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
	prompts.push_back(HelpPrompt("start", "menu"));

	return prompts;
}

HelpStyle ViewController::getHelpStyle()
{
	if(!mCurrentView)
		return GuiComponent::getHelpStyle();

	return mCurrentView->getHelpStyle();
}

// Trigger repopulating the list of valid systems
void ViewController::reloadSystemListView() const
{
	mSystemListView->populate();
}
