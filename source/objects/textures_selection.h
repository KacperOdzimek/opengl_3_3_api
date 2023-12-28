#pragma once
#include "graphics_abstraction.h"
#include "../common/opengl.h"

namespace graphics_abstraction
{
	namespace implementations
	{
		namespace opengl_3_3_api
		{
			struct textures_selection : public graphics_abstraction::textures_selection
			{
				friend class opengl_3_3_api;
				friend struct textures_selection_builder;

				textures_selection() {};

			protected:
				virtual void free()
				{
				}

				//TODO: textures types
				void bind_textures()
				{
					int offset = 0;
					for (auto& txt : textures)
					{
						internal::glActiveTexture(GL_TEXTURE0 + offset);
						internal::glBindTexture(
							GL_TEXTURE_2D, 
							static_cast<texture*>(txt)->id);
						offset++;
					}
				}
			};

			struct textures_selection_builder : public graphics_abstraction::textures_selection_builder
			{
				virtual object* build_impl(graphics_abstraction::api*& api)
				{
					graphics_abstraction::textures_selection* ts = new textures_selection{};
					ts->set_selection(textures);
					return ts;
				}
			};
		}
	}
}