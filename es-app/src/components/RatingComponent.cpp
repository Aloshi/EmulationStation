#include "components/RatingComponent.h"
#include "Renderer.h"
#include "Window.h"
#include "Util.h"
#include "resources/SVGResource.h"

RatingComponent::RatingComponent(Window* window) : GuiComponent(window)
{
	mFilledTexture = TextureResource::get(":/star_filled.svg", true);
	mUnfilledTexture = TextureResource::get(":/star_unfilled.svg", true);
	mValue = 0.5f;
	mSize << 64 * NUM_RATING_STARS, 64;
	updateVertices();
}

void RatingComponent::setValue(const std::string& value)
{
	if(value.empty())
	{
		mValue = 0.0f;
	}else{
		mValue = stof(value);
		if(mValue > 1.0f)
			mValue = 1.0f;
		else if(mValue < 0.0f)
			mValue = 0.0f;
	}

	updateVertices();
}

std::string RatingComponent::getValue() const
{
	// do not use std::to_string here as it will use the current locale
	// and that sometimes encodes decimals as commas
	std::stringstream ss;
	ss << mValue;
	return ss.str();
}

void RatingComponent::onSizeChanged()
{
	if(mSize.y() == 0)
		mSize[1] = mSize.x() / NUM_RATING_STARS;
	else if(mSize.x() == 0)
		mSize[0] = mSize.y() * NUM_RATING_STARS;

	auto filledSVG = dynamic_cast<SVGResource*>(mFilledTexture.get());
	auto unfilledSVG = dynamic_cast<SVGResource*>(mUnfilledTexture.get());

	if(mSize.y() > 0)
	{
		size_t heightPx = (size_t)round(mSize.y());
		if(filledSVG)
			filledSVG->rasterizeAt(heightPx, heightPx);
		if(unfilledSVG)
			unfilledSVG->rasterizeAt(heightPx, heightPx);
	}

	updateVertices();
}

void RatingComponent::updateVertices()
{
	const float numStars = NUM_RATING_STARS;

	const float h = round(getSize().y()); // is the same as a single star's width
	const float w = round(h * mValue * numStars);
	const float fw = round(h * numStars);

	mVertices[0].pos << 0.0f, 0.0f;
		mVertices[0].tex << 0.0f, 1.0f;
	mVertices[1].pos << w, h;
		mVertices[1].tex << mValue * numStars, 0.0f;
	mVertices[2].pos << 0.0f, h;
		mVertices[2].tex << 0.0f, 0.0f;

	mVertices[3] = mVertices[0];
	mVertices[4].pos << w, 0.0f;
		mVertices[4].tex << mValue * numStars, 1.0f;
	mVertices[5] = mVertices[1];

	mVertices[6] = mVertices[4];
	mVertices[7].pos << fw, h;
		mVertices[7].tex << numStars, 0.0f;
	mVertices[8] = mVertices[1];

	mVertices[9] = mVertices[6];
	mVertices[10].pos << fw, 0.0f;
		mVertices[10].tex << numStars, 1.0f;
	mVertices[11] = mVertices[7];
}

void RatingComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = roundMatrix(parentTrans * getTransform());
	Renderer::setMatrix(trans);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4ub(255, 255, 255, getOpacity());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].pos);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertices[0].tex);
	
	mFilledTexture->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	mUnfilledTexture->bind();
	glDrawArrays(GL_TRIANGLES, 6, 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glColor4ub(255, 255, 255, 255);

	renderChildren(trans);
}

bool RatingComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		mValue += 1.f / NUM_RATING_STARS;
		if(mValue > 1.0f)
			mValue = 0.0f;

		updateVertices();
	}

	return GuiComponent::input(config, input);
}

void RatingComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "rating");
	if(!elem)
		return;

	bool imgChanged = false;
	if(properties & PATH && elem->has("filledPath"))
	{
		mFilledTexture = TextureResource::get(elem->get<std::string>("filledPath"), true);
		imgChanged = true;
	}
	if(properties & PATH && elem->has("unfilledPath"))
	{
		mUnfilledTexture = TextureResource::get(elem->get<std::string>("unfilledPath"), true);
		imgChanged = true;
	}

	if(imgChanged)
		onSizeChanged();
}

std::vector<HelpPrompt> RatingComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", "add star"));
	return prompts;
}
