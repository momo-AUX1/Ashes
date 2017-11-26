/**
*\file
*	Viewport.h
*\author
*	Sylvain Doremus
*/
#ifndef ___RenderLib_Viewport_HPP___
#define ___RenderLib_Viewport_HPP___
#pragma once

#include <Renderer/Viewport.hpp>

#include <Utils/Angle.hpp>

namespace render
{
	/**
	*\brief
	*	Gère le viewport d'un framebuffer.
	*/
	class Viewport
	{
	public:
		/**
		*\brief
		*	Constructeur.
		*\param[in] size
		*	Les dimensions du viewport.
		*/
		Viewport( utils::IVec2 const & size )noexcept;
		/**
		*\brief
		*	Redimensionne le viewport.
		*\param[in] size
		*	Les dimensions du viewport.
		*/
		void resize( utils::IVec2 const & size )noexcept;
		/**
		*\brief
		*	Définit le viewport en tant que projection orthographique.
		*\param[in] left, right
		*	Les positions des plans gauche et droite.
		*\param[in] top, bottom
		*	Les positions des plans haut et bas.
		*\param[in] near, far
		*	Les positions des plans proche et éloigné.
		*/
		void ortho( float left
			, float right
			, float top
			, float bottom
			, float near
			, float far )noexcept;
		/**
		*\brief
		*	Définit l'angle d'ouverture vertical.
		*\remarks
		*	Recalcule la matrice de projection.
		*\param[in] fovy
		*	La nouvelle valeur.
		*/
		void fovY( utils::Angle const & fovy )noexcept;
		/**
		*\return
		*	La matrice de projection du viewport.
		*/
		inline utils::Mat4 const & transform()const noexcept
		{
			return m_projection;
		}
		/**
		*\return
		*	Les dimensions du viewport.
		*/
		inline utils::IVec2 const & size()const noexcept
		{
			return m_size;
		}
		/**
		*\return
		*	L'angle d'ouverture vertical.
		*/
		inline utils::Angle const & fovY()const noexcept
		{
			return m_fovy;
		}
		/**
		*\return
		*	\p true si le viewport a changé.
		*/
		inline bool changed()const noexcept
		{
			auto result = m_changed;
			m_changed = false;
			return result;
		}
		/**
		*\return
		*	Le viewport du renderer.
		*/
		inline renderer::Viewport const & viewport()const noexcept
		{
			return m_viewport;
		}

	private:
		//! Les dimensions du viewport.
		utils::IVec2 m_size;
		//! La matrice de projection.
		utils::Mat4 m_projection;
		//! L'angle d'ouverture verticale.
		utils::Angle m_fovy;
		//! Le viewport du renderer.
		renderer::Viewport m_viewport;
		//! Dit si le viewport a changé.
		mutable bool m_changed{ true };
	};
}

#endif
