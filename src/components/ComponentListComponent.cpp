#include "ComponentListComponent.h"
#include "../Log.h"
#include "../Renderer.h"

#define INITIAL_CELL_SIZE 12

ComponentListComponent::ComponentListComponent(Window* window, Vector2u gridDimensions) : GuiComponent(window), mGrid(NULL), mColumnWidths(NULL), mRowHeights(NULL)
{
	mEntries.reserve(gridDimensions.x*gridDimensions.y);
	makeCells(gridDimensions);
}

void ComponentListComponent::makeCells(Vector2u size)
{
	if(mGrid)
		delete[] mGrid;
	if(mColumnWidths)
		delete[] mColumnWidths;
	if(mRowHeights)
		delete[] mRowHeights;
	
	mGridSize = size;
	mGrid = new ComponentEntry*[size.x * size.y];
	std::fill(mGrid, mGrid + (size.x * size.y), (ComponentEntry*)NULL);

	mColumnWidths = new unsigned int[size.x];
	std::fill(mColumnWidths, mColumnWidths + size.x, INITIAL_CELL_SIZE);
	
	mRowHeights = new unsigned int[size.y];
	std::fill(mRowHeights, mRowHeights + size.y, INITIAL_CELL_SIZE);

	updateSize();
	resetCursor();
}

void ComponentListComponent::setEntry(Vector2u pos, Vector2u size, GuiComponent* component, bool canFocus, AlignmentType align, 
	Vector2<bool> autoFit, UpdateBehavior updateType)
{
	if(pos.x > mGridSize.x || pos.y > mGridSize.y)
	{
		LOG(LogError) << "Tried to set entry beyond grid size!";
		return;
	}

	if(component == NULL)
	{
		LOG(LogError) << "Tried to add NULL component to ComponentList!";
		return;
	}

	ComponentEntry entry(Rect(pos.x, pos.y, size.x, size.y), component, updateType, canFocus, align);
	
	mEntries.push_back(entry);

	for(unsigned int y = pos.y; y < pos.y + size.y; y++)
	{
		for(unsigned int x = pos.x; x < pos.x + size.x; x++)
		{
			setCell(x, y, &mEntries.back());
		}
	}

	if(component->getParent() != NULL)
		LOG(LogError) << "ComponentListComponent ruining an existing parent-child relationship! Call a social worker!";
	component->setParent(this);

	if(!cursorValid() && canFocus)
		mCursor = (Vector2i)pos;

	//update the column width and row height
	if(autoFit.x && getColumnWidth(pos.x) < component->getSize().x)
		setColumnWidth(pos.x, component->getSize().x);
	if(autoFit.y && getRowHeight(pos.y) < component->getSize().y)
		setRowHeight(pos.y, component->getSize().y);

	component->setOffset(getCellOffset(pos));
}

void ComponentListComponent::setRowHeight(int row, unsigned int size)
{
	mRowHeights[row] = size;
	updateSize();
}

void ComponentListComponent::setColumnWidth(int col, unsigned int size)
{
	mColumnWidths[col] = size;
	updateSize();
}

unsigned int ComponentListComponent::getRowHeight(int row) { return mRowHeights[row]; }
unsigned int ComponentListComponent::getColumnWidth(int col) { return mColumnWidths[col]; }

Vector2i ComponentListComponent::getCellOffset(Vector2u pos)
{
	Vector2i offset;

	for(unsigned int y = 0; y < pos.y; y++)
		offset.y += getRowHeight(y);
	
	for(unsigned int x = 0; x < pos.x; x++)
		offset.x += getColumnWidth(x);

	ComponentEntry* entry = getCell(pos.x, pos.y);

	Vector2u gridSize;
	for(unsigned int x = pos.x; x < pos.x + entry->box.size.x; x++)
		gridSize.x += getColumnWidth(x);
	for(unsigned int y = pos.y; y < pos.y + entry->box.size.y; y++)
		gridSize.y += getRowHeight(y);

	//if AlignCenter, add half of cell width - half of control width
	if(entry->alignment == AlignCenter)
		offset.x += gridSize.x / 2 - entry->component->getSize().x / 2;

	//if AlignRight, add cell width - control width
	if(entry->alignment == AlignRight)
		offset.x += gridSize.x - entry->component->getSize().x;

	//always center on the Y axis
	offset.y += gridSize.y / 2 - entry->component->getSize().y / 2;

	return offset;
}

void ComponentListComponent::setCell(unsigned int x, unsigned int y, ComponentEntry* entry)
{
	if(x < 0 || y < 0 || x >= mGridSize.x || y >= mGridSize.y)
	{
		LOG(LogError) << "Invalid setCell - position " << x << ", " << y << " out of bounds!";
		return;
	}

	mGrid[y * mGridSize.x + x] = entry;
}

ComponentListComponent::ComponentEntry* ComponentListComponent::getCell(unsigned int x, unsigned int y)
{
	if(x < 0 || y < 0 || x >= mGridSize.x || y >= mGridSize.y)
	{
		LOG(LogError) << "Invalid getCell - position " << x << ", " << y << " out of bounds!";
		return NULL;
	}

	return mGrid[y * mGridSize.x + x];
}

void ComponentListComponent::updateSize()
{
	mSize = Vector2u(0, 0);
	for(unsigned int x = 0; x < mGridSize.x; x++)
		mSize.x += getColumnWidth(x);
	for(unsigned int y = 0; y < mGridSize.y; y++)
		mSize.y += getRowHeight(y);
}

void ComponentListComponent::updateComponentOffsets()
{
	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		iter->component->setOffset(getCellOffset((Vector2u)iter->box.pos));
	}
}

bool ComponentListComponent::input(InputConfig* config, Input input)
{
	if(cursorValid() && getCell(mCursor.x, mCursor.y)->component->input(config, input))
		return true;

	if(!input.value)
		return false;

	if(config->isMappedTo("down", input))
	{
		moveCursor(Vector2i(0, 1));
		return true;
	}
	if(config->isMappedTo("up", input))
	{
		moveCursor(Vector2i(0, -1));
		return true;
	}

	return false;
}

void ComponentListComponent::resetCursor()
{
	if(mEntries.size() == 0)
	{
		mCursor = Vector2i(-1, -1);
		return;
	}

	mCursor = mEntries.at(0).box.pos;
}

void ComponentListComponent::moveCursor(Vector2i dir)
{
	if(dir.x != 0 && dir.y != 0)
	{
		LOG(LogError) << "Invalid cursor move dir!";
		return;
	}

	if(!cursorValid())
	{
		resetCursor();
		if(!cursorValid())
			return;
	}

	Vector2i origCursor = mCursor;
	
	Vector2i searchAxis(dir.x == 0, dir.y == 0);
	
	while(mCursor.x >= 0 && mCursor.y >= 0 && mCursor.x < (int)mGridSize.x && mCursor.y < (int)mGridSize.y)
	{
		mCursor = mCursor + dir;

		Vector2i curDirPos = mCursor;

		//spread out on search axis+
		while(mCursor.x < (int)mGridSize.x && mCursor.y < (int)mGridSize.y)
		{
			if(cursorValid() && getCell(mCursor.x, mCursor.y)->canFocus)
				return;

			mCursor += searchAxis;
		}

		//now again on search axis-
		mCursor = curDirPos;
		while(mCursor.x >= 0 && mCursor.y >= 0)
		{
			if(cursorValid() && getCell(mCursor.x, mCursor.y)->canFocus)
				return;

			mCursor -= searchAxis;
		}
	}

	//failed to find another focusable element in this direction
	mCursor = origCursor;
}

bool ComponentListComponent::cursorValid()
{
	if(mCursor.x < 0 || mCursor.y < 0 || mCursor.x >= (int)mGridSize.x || mCursor.y >= (int)mGridSize.y)
		return false;

	return getCell(mCursor.x, mCursor.y) != NULL;
}

void ComponentListComponent::update(int deltaTime)
{
	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		if(iter->updateType == UpdateAlways)
		{
			iter->component->update(deltaTime);
			continue;
		}

		if(iter->updateType == UpdateFocused && cursorValid() && getCell(mCursor.x, mCursor.y)->component == iter->component)
		{
			iter->component->update(deltaTime);
			continue;
		}
	}
}

void ComponentListComponent::onRender()
{
	Renderer::drawRect(0, 0, getSize().x, getSize().y, 0xFFFFFFAA);

	for(auto iter = mEntries.begin(); iter != mEntries.end(); iter++)
	{
		iter->component->render();
	}

	//draw cell outlines
	/*Vector2i pos;
	for(unsigned int x = 0; x < mGridSize.x; x++)
	{
		for(unsigned int y = 0; y < mGridSize.y; y++)
		{
			Renderer::drawRect(pos.x, pos.y, getColumnWidth(x), 2, 0x000000AA);
			Renderer::drawRect(pos.x, pos.y, 2, getRowHeight(y), 0x000000AA);
			Renderer::drawRect(pos.x + getColumnWidth(x), pos.y, 2, getRowHeight(y), 0x000000AA);
			Renderer::drawRect(pos.x, pos.y + getRowHeight(y) - 2, getColumnWidth(x), 2, 0x000000AA);

			pos.y += getRowHeight(y);
		}

		pos.y = 0;
		pos.x += getColumnWidth(x);
	}*/

	//draw cursor
	if(cursorValid())
	{
		ComponentEntry* entry = getCell(mCursor.x, mCursor.y);
		Renderer::drawRect(entry->component->getOffset().x, entry->component->getOffset().y, 4, 4, 0xFF0000FF);
		Renderer::drawRect(entry->component->getOffset().x, entry->component->getOffset().y, entry->component->getSize().x, entry->component->getSize().y, 0x0000AA88);
	}
}

void ComponentListComponent::onOffsetChanged()
{
	updateComponentOffsets();
}
