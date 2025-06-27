#pragma once

#include "Rendering/Renderer.h"
#include "glad/gl.h"

constexpr u32 MAX_QUADS    = 1024; // Everything we render is a quad.
constexpr u32 MAX_VERTICES = MAX_QUADS * 4;
constexpr u32 MAX_INDICES  = MAX_QUADS * 6;

namespace Velox {

struct Pipeline {
    u32 id;
    i32 GLDrawType = GL_TRIANGLES;

    u32 vertexCount = 0;
    u32 indexCount = 0;
    u32 indices[MAX_INDICES];

    u32 vao;
    u32 vbo;
    u32 ibo;

    void Use(u32 ubo)
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    }

    void ClearFrameData()
    {
        vertexCount = 0;
        indexCount = 0;
    }

    void DeInit()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &vao);
        glDeleteBuffers(1, &ibo);
    }
};

struct LinePipeline : Pipeline {
    Velox::LineVertex vertices[MAX_VERTICES];

    void Init(u32 id)
    {
        this->id = id;

        GLDrawType = GL_LINES;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Line Attributes");

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, vbo, -1, "Line Vertex Buffer");

        // Vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Velox::LineVertex), (void*)offsetof(Velox::LineVertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Velox::LineVertex), (void*)offsetof(Velox::LineVertex, color));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, ibo, -1, "Line Index Buffer");

        glBindVertexArray(0);
    }

    void CopyData()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), &indices);
    }
};

struct TexturedQuadPipeline : Pipeline {
    Velox::TextureVertex vertices[MAX_VERTICES];

    void Init(u32 id)
    {
        this->id = id;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Textured Quad Attributes");

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, vbo, -1, "Textured Quad Vertex Buffer");

        // Vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Velox::TextureVertex), (void*)offsetof(Velox::TextureVertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Velox::TextureVertex), (void*)offsetof(Velox::TextureVertex, color));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Velox::TextureVertex), (void*)offsetof(Velox::TextureVertex, uv));
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, ibo, -1, "Textured Quad Index Buffer");

        glBindVertexArray(0);
    }

    void CopyData()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), &indices);
    }
};

struct FontPipeline : Pipeline {
    Velox::FontVertex vertices[MAX_VERTICES];

    void Init(u32 id)
    {
        this->id = id;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Font Attributes");

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, vbo, -1, "Font Vertex Buffer");

        // Vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex), 
                (void*)offsetof(Velox::FontVertex, color));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, uv));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, fontWeightBias));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, outlineColor));
        glEnableVertexAttribArray(4);

        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, outlineWidth));
        glEnableVertexAttribArray(5);

        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, shadowColor));
        glEnableVertexAttribArray(6);

        glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(Velox::FontVertex),
                (void*)offsetof(Velox::FontVertex, shadowOffset));
        glEnableVertexAttribArray(7);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
        glObjectLabel(GL_BUFFER, ibo, -1, "Font Index Buffer");

        glBindVertexArray(0);
    }

    void CopyData()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), &indices);
    }
};


}

