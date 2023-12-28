#pragma once
#include "graphics_abstraction.h"
#include "opengl.h"

#include "../objects/shader.h"
#include "../objects/buffer.h"
#include "../objects/vertex_layout.h"
#include "../objects/texture.h"
#include "../objects/textures_selection.h"
#include "../objects/framebuffer.h"

#include "mappings.h"

namespace graphics_abstraction
{
	namespace implementations
	{
		namespace opengl_3_3_api
		{
			class opengl_3_3_api : public graphics_abstraction::api
			{
				friend buffer;

			private:
				bool can_draw = 0;
				internal::GLuint VAO = 0;
				framebuffer* default_framebuffer = nullptr;

			public:
				opengl_3_3_api()
				{
					internal::gladLoadGL();
					//Should be never unbinded
					internal::glGenVertexArrays(1, &VAO);
					internal::glBindVertexArray(VAO);

					internal::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					default_framebuffer = new framebuffer;
					default_framebuffer->id = 0;
				}
				~opengl_3_3_api()
				{
					internal::glDeleteVertexArrays(1, &VAO);
				}

				virtual void get_errors_impl()
				{
					int error = internal::glGetError();
					if (error != 0)
						errors_queue.push_back({ error_source::implementation, (uint16_t)error, {} });
				}

				//Builders requests
				virtual framebuffer_builder* create_framebuffer_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::framebuffer_builder;
				}
				virtual buffer_builder* create_buffer_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::buffer_builder;
				}
				virtual shader_builder* create_shader_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::shader_builder;
				}
				virtual vertex_layout_builder* create_vertex_layout_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::vertex_layout_builder;
				}
				virtual texture_builder* create_texture_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::texture_builder;
				}
				virtual textures_selection_builder* create_textures_selection_builder()
				{
					return new graphics_abstraction::implementations::opengl_3_3_api::textures_selection_builder;
				}

			protected:
				void bind_framebuffer()
				{
					if (current_pipeline.framebuffer == nullptr)
						can_draw = false;
					else
						internal::glBindFramebuffer(GL_FRAMEBUFFER, static_cast<framebuffer*>(current_pipeline.framebuffer)->id);
				}

				void bind_shader()
				{
					if (current_pipeline.shader == nullptr)
						can_draw = false;
					else
						internal::glUseProgram(static_cast<shader*>(current_pipeline.shader)->id);
				}

				void bind_vbo()
				{
					if (current_pipeline.vertex_buffer == nullptr)
						can_draw = false;
					else
						internal::glBindBuffer(GL_ARRAY_BUFFER, static_cast<buffer*>(current_pipeline.vertex_buffer)->id);
				}

				void bind_textures()
				{
					if (current_pipeline.textures_selection != nullptr)
						static_cast<textures_selection*>(current_pipeline.textures_selection)->bind_textures();
				}

				void update_VAO_indicies_buffer(buffer* ib)
				{
					if (ib == nullptr)
						internal::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					else
						internal::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id);
				}

				void update_VAO_vertex_layout(vertex_layout* vl)
				{
					if (vl == nullptr)
					{
						can_draw = false;
						return;
					}

					int size_in_bytes = vl->get_vertex_size();
					int index = 0;
					int offset = 0;

					for (auto& component : vl->get_vertex_components())
					{
						int amount = 0;
						int type = 0;

						switch (component)
						{
						case data_type::Bool:
							amount = 1;
							type = GL_BOOL;
							break;
						case data_type::Int:
							amount = 1;
							type = GL_INT;
							break;
						case data_type::Float:
							amount = 1;
							type = GL_FLOAT;
							break;
						case data_type::vec2:
							amount = 2;
							type = GL_FLOAT;
							break;
						case data_type::vec3:
							amount = 3;
							type = GL_FLOAT;
							break;
						case data_type::vec4:
							amount = 4;
							type = GL_FLOAT;
							break;
						}

						internal::glVertexAttribPointer(index, amount, type, GL_FALSE, size_in_bytes + vl->additional_bytes, (void*)(uintptr_t)(offset));
						internal::glEnableVertexAttribArray(index);

						offset += amount * 4;

						index++;
					}

					for (int i = (int)vl->get_vertex_components().size(); i != 16; i++)
						internal::glDisableVertexAttribArray(i);
				}

				virtual bool apply_bindings_impl()
				{
					can_draw = true;
					bind_framebuffer();
					bind_shader();
					internal::glBindVertexArray(VAO);
					bind_vbo();
					update_VAO_indicies_buffer(static_cast<buffer*>(current_pipeline.indices_buffer));
					update_VAO_vertex_layout(static_cast<vertex_layout*>(current_pipeline.vertex_layout));
					bind_textures();
					return can_draw;
				}

			public:
				virtual bool draw_impl(draw_args& draw_args)
				{
					if (!can_draw)
						return false;

					apply_bindings_impl();	//Running functions on objects can change opengl bindings 
											//(eg. shader::set_uniform_value which change active shader via glUseProgram)
											//So we have to ensure glState is congruent to current_pipeline

					switch (draw_args.draw_type)
					{
					case draw_types::array:
					{
						internal::glDrawArrays(
							internal::primitive_to_opengl(draw_args.primitive),
							draw_args.args.array_draw_args.first_vertex_index, 
							draw_args.args.array_draw_args.vertices_amount
						);
					} break;
					case draw_types::instanced_array:
					{
						internal::glDrawArraysInstanced(
							internal::primitive_to_opengl(draw_args.primitive),
							draw_args.args.instanced_array_draw_args.array_args.first_vertex_index,
							draw_args.args.instanced_array_draw_args.array_args.vertices_amount,
							draw_args.args.instanced_array_draw_args.instances_amount
						);
					} break;
					case draw_types::indexed:
					{
						internal::glDrawElements(
							internal::primitive_to_opengl(draw_args.primitive),
							draw_args.args.indexed_draw_args.indices_amount,
							GL_UNSIGNED_INT,
							0
						);
					} break;
					case draw_types::instanced_indexed:
					{
						internal::glDrawElementsInstanced(
							internal::primitive_to_opengl(draw_args.primitive),
							draw_args.args.instanced_indexed_draw_args.indexed_args.indices_amount,
							GL_UNSIGNED_INT,
							0,
							draw_args.args.instanced_indexed_draw_args.instances_amount
						);
					} break;
					}
					return true;
				};

				virtual bool clear_current_framebuffer_color_buffer(float r, float g, float b, float a)
				{
					internal::glClearColor(r, g, b, a);
					internal::glClear(GL_COLOR_BUFFER_BIT);
					return true;
				}

				virtual bool clear_current_framebuffer_depth_buffer()
				{
					internal::glClear(GL_DEPTH_BUFFER_BIT);
					return true;
				}

				virtual bool clear_current_framebuffer_stencil_buffer()
				{
					internal::glClear(GL_STENCIL_BUFFER_BIT);
					return true;
				}

				virtual bool set_screen_size(int width, int height)
				{
					internal::glViewport(0, 0, width, height);
					return true;
				}

				virtual framebuffer* get_default_framebuffer()
				{
					return default_framebuffer;
				}

				virtual bool set_enabled(functionalities functionality, bool enabled)
				{
					switch (functionality)
					{
					case graphics_abstraction::functionalities::wireframe_mode:
						if (enabled)
							internal::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						else
							internal::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						break;
					case graphics_abstraction::functionalities::depth_testing:
						if (enabled)
							internal::glEnable(GL_DEPTH_TEST);
						else
							internal::glDisable(GL_DEPTH_TEST);
						break;
					case graphics_abstraction::functionalities::transparency:
						if (enabled)
							internal::glEnable(GL_BLEND);
						else
							internal::glDisable(GL_BLEND);
						break;
					default:
						break;
					}
					return 1;
				}
			};
		}
	}
};