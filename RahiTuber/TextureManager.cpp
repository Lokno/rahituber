#include "TextureManager.h"

#include "file_browser_modal.h"

#include <thread>

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_JPEG
//#define STBI_NO_PNG
//#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM 
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

SDL_Texture* TextureManager::LoadTextureFromFile(SDL_Renderer *renderer, const char *file)
{
	int width, height, channels;
	unsigned char *data = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);

	if (!data)
	{
		SDL_Log("Failed to load image %s: %s", file, stbi_failure_reason());
		return nullptr;
	}

	bool needsResize = false;

	int newWidth, newHeight;

	if (_max_texture_size > 0 && _max_texture_size > 0)
	{
		if (width > _max_texture_size || height > _max_texture_size)
		{
			needsResize = true;
			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			if (width > height)
			{
				newHeight = static_cast<int>(_max_texture_size / aspectRatio);
				newWidth = _max_texture_size;
			}
			else
			{
				newWidth = static_cast<int>(_max_texture_size * aspectRatio);
				newHeight = _max_texture_size;
			}
		}
	}

	if (needsResize)
	{
		unsigned char* resizedData = new unsigned char[newWidth * newHeight * 4];
		stbir_resize_uint8_linear(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBIR_RGBA);
		stbi_image_free(data);
		data = resizedData;
		width = newWidth;
		height = newHeight;

		stbi_image_free(data);
	}

	SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, data, width * sizeof(uint32_t));
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);

	if (!texture)
	{
		SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
		return nullptr;
	}

	return texture;
}

void TextureManager::SetSmooth(SDL_Texture* texture, int scaleFiltering)
{
	if (texture == nullptr)
		return;

    std::scoped_lock loadLock(_loadMutex);
	// Third option SDL_ScaleMode::SDL_SCALEMODE_PIXELART is supported in SDL3 and should be considered as an option 
	SDL_SetTextureScaleMode(texture, scaleFiltering == 0 ? SDL_ScaleMode::SDL_SCALEMODE_NEAREST : SDL_ScaleMode::SDL_SCALEMODE_LINEAR);
}

void TextureManager::LoadIcons(const std::string& appLocation)
{
	if (_icons.count(ICON_ANIM) == 0)
		LoadIcon(appLocation + "res/anim.png", _icons[ICON_ANIM]);
	if (_icons.count(ICON_EMPTY) == 0)
		LoadIcon(appLocation + "res/empty.png", _icons[ICON_EMPTY]);
	if (_icons.count(ICON_UP) == 0)
		LoadIcon(appLocation + "res/arrowup.png", _icons[ICON_UP]);
	if (_icons.count(ICON_DN) == 0)
		LoadIcon(appLocation + "res/arrowdn.png", _icons[ICON_DN]);
	if (_icons.count(ICON_EDIT) == 0)
		LoadIcon(appLocation + "res/edit.png", _icons[ICON_EDIT]);
	if (_icons.count(ICON_DEL) == 0)
		LoadIcon(appLocation + "res/delete.png", _icons[ICON_DEL]);
	if (_icons.count(ICON_DUPE) == 0)
		LoadIcon(appLocation + "res/duplicate.png", _icons[ICON_DUPE]);
	if (_icons.count(ICON_NEWFILE) == 0)
		LoadIcon(appLocation + "res/new_file.png", _icons[ICON_NEWFILE]);
	if (_icons.count(ICON_OPEN) == 0)
		LoadIcon(appLocation + "res/open.png", _icons[ICON_OPEN]);
	if (_icons.count(ICON_SAVE) == 0)
		LoadIcon(appLocation + "res/save.png", _icons[ICON_SAVE]);
	if (_icons.count(ICON_SAVEAS) == 0)
		LoadIcon(appLocation + "res/save_as.png", _icons[ICON_SAVEAS]);
	if (_icons.count(ICON_MAKEPORTABLE) == 0)
		LoadIcon(appLocation + "res/make_portable.png", _icons[ICON_MAKEPORTABLE]);
	if (_icons.count(ICON_RELOAD) == 0)
		LoadIcon(appLocation + "res/reload.png", _icons[ICON_RELOAD]);
	if (_icons.count(ICON_NEWLAYER) == 0)
		LoadIcon(appLocation + "res/new_layer.png", _icons[ICON_NEWLAYER]);
	if (_icons.count(ICON_NEWFOLDER) == 0)
		LoadIcon(appLocation + "res/new_folder.png", _icons[ICON_NEWFOLDER]);
	if (_icons.count(ICON_STATES) == 0)
		LoadIcon(appLocation + "res/states.png", _icons[ICON_STATES]);
	if (_icons.count(ICON_RESET) == 0)
		LoadIcon(appLocation + "res/reset.png", _icons[ICON_RESET]);
	if (_icons.count(ICON_PLUS) == 0)
		LoadIcon(appLocation + "res/plus.png", _icons[ICON_PLUS]);
	if (_icons.count(ICON_LOCK_OPEN) == 0)
		LoadIcon(appLocation + "res/lock_open.png", _icons[ICON_LOCK_OPEN]);
	if (_icons.count(ICON_LOCK_CLOSED) == 0)
		LoadIcon(appLocation + "res/lock_closed.png", _icons[ICON_LOCK_CLOSED]);
	if (_icons.count(ICON_EYE_OPEN) == 0)
		LoadIcon(appLocation + "res/eye_open.png", _icons[ICON_EYE_OPEN]);
	if (_icons.count(ICON_EYE_CLOSED) == 0)
		LoadIcon(appLocation + "res/eye_closed.png", _icons[ICON_EYE_CLOSED]);
	if (_icons.count(ICON_PIN) == 0)
		LoadIcon(appLocation + "res/pin.png", _icons[ICON_PIN]);
	if (_icons.count(ICON_PIN_OFF) == 0)
		LoadIcon(appLocation + "res/pin_off.png", _icons[ICON_PIN_OFF]);
}

SDL_Texture* TextureManager::GetTexture(const std::string& path, void* caller, std::string* errString)
{
	if(errString != nullptr)
		*errString = "";

	if (path.empty())
		return nullptr;

	SDL_Texture* out = nullptr;

	while (_textures.count(path) && _textures[path].busyLoading)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	if ( _textures[path].tex != nullptr)
	{
		_textures[path].refHolders[caller] = true;
		out = _textures[path].tex.get();
	}
	else
	{
		_textures[path].busyLoading = true;

		bool success = LoadTexture(path, caller, errString);
		if (success)
			out = _textures[path].tex.get();
		else
			_textures.erase(path);
	}

	return out;
}

bool TextureManager::LoadIcon(const std::string& path, SDL_Texture*& storage)
{
	storage = nullptr;
	int tries = 5;
	while (tries > 0)
	{
		bool success = false;
		std::string err = "";

		storage = TextureManager::LoadTextureFromFile(_renderer, path.c_str());
		success = storage != nullptr;

		if (success)
		{
			return true;
		}
		else
		{
			err = ": " + std::string(SDL_GetError());
		}

		tries--;
	}

	return false;
}

bool TextureManager::LoadTexture(const std::string& path, void* caller, std::string* errString)
{
	int tries = 5;

	while (tries > 0)
	{
		bool success = false;
		std::string err = "";
		try
		{
			std::unique_ptr<SDL_Texture> loadingTex(TextureManager::LoadTextureFromFile(_renderer, path.c_str()));

			if(loadingTex != nullptr)
			{
				std::scoped_lock loadLock(_loadMutex);
				_textures[path].refHolders[caller] = true;
				_textures[path].tex = std::move(loadingTex);
				success = true;
			}
		}
		catch (const std::exception& exc)
		{
			err = ": " + std::string(exc.what());
		}

		if (success)
		{
			_textures[path].busyLoading = false;
			return true;
		}
		else
		{
			if (errString)
			{
				fs::path fpath = path;
				*errString = "Failed to load " + fpath.filename().string() + ": ";
				std::error_code ec;
				if (fs::exists(path, ec))
				{
					*errString += "Load error" + err;
					Vector2i imgDim = GetDimensions(path.c_str());
					if (imgDim.x > _max_texture_size || imgDim.y > _max_texture_size)
					{
						*errString += " - Too large Size: " + std::to_string(imgDim.x) + "x" + std::to_string(imgDim.y);
						return false;
					}
				}
				else
				{
					*errString += "File does not exist";
					return false;
				}
			}
			
		}
		tries--;
	}

	return false;
}

void TextureManager::UnloadTexture(const std::string& path, void* caller)
{
	if (_textures.count(path) != 0)
	{
		if (_textures[path].refHolders.size() <= 1)
		{
			_textures[path].tex = nullptr;
			_textures.erase(path);
		}
		else if (_textures[path].refHolders.count(caller))
		{
			_textures[path].refHolders.erase(caller);
		}
	}
}

void TextureManager::Reset()
{
	std::scoped_lock loadLock(_loadMutex);
	auto it = _textures.begin();
	for (; it != _textures.end(); it++)
	{
		(*it).second.refHolders.clear();
		(*it).second.tex = nullptr;
	}
	_textures.clear();
}

SDL_Texture* TextureManager::GetIcon(IconID id)
{
	if (_icons.count(id))
		return _icons[id];

	return nullptr;
}

Vector2i TextureManager::GetDimensions(const char* path) 
{
	Vector2i dim;
	int n, ok = 0;
    ok = stbi_info(path, &dim.x, &dim.y, &n);
	return ok ? dim : Vector2i(0, 0);
}

void TextureManager::SetRenderer(SDL_Renderer* renderer)
{
	_renderer = renderer;
    SDL_PropertiesID props = SDL_GetRendererProperties(_renderer);
	if( SDL_HasProperty(props, "SDL.renderer.max_texture_size"))
    {
        _max_texture_size = SDL_GetNumberProperty(props, "SDL.renderer.max_texture_size", 0);
    }
}