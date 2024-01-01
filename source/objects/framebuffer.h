#pragma once
#include "graphics_abstraction.h"
#include "../common/opengl.h"

#include "../common/mappings.h"

namespace graphics_abstraction
{
	namespace implementations
	{
		namespace opengl_3_3_api
		{
			struct framebuffer : public graphics_abstraction::framebuffer
			{
				internal::GLuint id = 0;
				framebuffer()
				{					
					internal::glGenFramebuffers(1, &id);
					internal::glBindFramebuffer(GL_FRAMEBUFFER, id);
				}

			protected:
				virtual void set_color_buffer_impl(uint8_t number)
				{
					switch (color_buffers.at(number)->texture_type)
					{
					case graphics_abstraction::texture_type::texture_1d:
						internal::glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							GL_TEXTURE_1D, static_cast<texture*>(color_buffers.at(number))->id, 0);
						break;
					case graphics_abstraction::texture_type::texture_2d:
						internal::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							GL_TEXTURE_2D, static_cast<texture*>(color_buffers.at(number))->id, 0);
						break;
					case graphics_abstraction::texture_type::texture_3d:
						internal::glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							GL_TEXTURE_3D, static_cast<texture*>(color_buffers.at(number))->id, 0, 0);
						break;
					case graphics_abstraction::texture_type::renderbuffer:
						internal::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							GL_RENDERBUFFER, static_cast<texture*>(color_buffers.at(number))->id);
						return;
					}
				}

				virtual void set_depth_stencil_buffer_impl()
				{
					switch (depth_stencil_buffer->texture_type)
					{
					case graphics_abstraction::texture_type::texture_1d:
						internal::glFramebufferTexture1D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_1D, static_cast<texture*>(depth_stencil_buffer)->id, 0);
						break;
					case graphics_abstraction::texture_type::texture_2d:
						internal::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, static_cast<texture*>(depth_stencil_buffer)->id, 0);
						break;
					case graphics_abstraction::texture_type::texture_3d:
						internal::glFramebufferTexture3D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_3D, static_cast<texture*>(depth_stencil_buffer)->id, 0, 0);
						break;
					case graphics_abstraction::texture_type::renderbuffer:
						internal::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, static_cast<texture*>(depth_stencil_buffer)->id);
						return;
					}
				}

			protected:
				virtual void free()
				{
					if (id != 0)
						internal::glDeleteFramebuffers(1, &id);
				}
			};

			struct framebuffer_builder : public graphics_abstraction::framebuffer_builder
			{
				virtual object* build_impl(graphics_abstraction::api*& api)
				{
					auto fb = new framebuffer{};
					for (auto& text : color_buffers)
					{
						fb->set_color_buffer(text.first, text.second);
					}
					fb->set_depth_stencil_buffer(depth_stencil_buffer);
					return fb;
				}
			};
		}
	}
}