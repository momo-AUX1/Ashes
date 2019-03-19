/*
See LICENSE file in root folder
*/
#ifndef ___Utils_FileUtils_H___
#define ___Utils_FileUtils_H___
#pragma once

#include "UtilsPrerequisites.hpp"

#include <fstream>

namespace utils
{
	using StringArray = ashes::StringArray;
	/**
	*\~english
	*\brief
	*	List all files in a directory, recursively or not.
	*\param[in] folderPath
	*	The directory path.
	*\param[out] files
	*	Receives the files list.
	*\param[in] recursive
	*	Tells if search must be recursive.
	*\return
	*	\p true if the directory is listed.
	*\~french
	*\brief
	*	Liste tous les fichiers d'un répertoire, récursivement ou pas.
	*\param[in] folderPath
	*	Le chemin du dossier.
	*\param[out] files
	*	Reçoit la liste des fichiers.
	*\param[in] recursive
	*	Définit si la recherche doit être récursive ou pas.
	*\return
	*	\p true si le dossier a été listé.
	*/
	bool listDirectoryFiles( std::string const & folderPath
		, StringArray & files
		, bool recursive = false );
	/**
	*\~french
	*\brief
	*	Charge le contenu d'un fichier texte dans un tableau d'octets.
	*\param[in] file
	*	Le chemin d'accès au fichier.
	*\return
	*	Le contenu du fichier.
	*\~english
	*\brief
	*	Loads the content of a text file into a byte array.
	*\param[in] file
	*	The file path.
	*\return
	*	The file content.
	*/
	std::string dumpTextFile( std::string const & file );
	/**
	*\~french
	*\brief
	*	Charge le contenu d'un fichier binaire dans un tableau d'octets.
	*\param[in] file
	*	Le chemin d'accès au fichier.
	*\return
	*	Le contenu du fichier.
	*\~english
	*\brief
	*	Loads the content of a binary file into a byte array.
	*\param[in] file
	*	The file path.
	*\return
	*	The file content.
	*/
	ashes::ByteArray dumpBinaryFile( std::string const & file );
	/**
	*\~french
	*\return
	*	Le dossier d'exécution du programme.
	*\~english
	*\return
	*	The executable folder.
	*/
	std::string getExecutableDirectory();
	/**
	*\~french
	*\brief
	*	Récupère le dossier parent du chemin donné.
	*\param[in] path
	*	Le chemin.
	*\return
	*	Le dossier parent.
	*\~english
	*\brief
	*	Retrieves the given path's parent folder.
	*\param[in] path
	*	The path.
	*\return
	*	The parent folder.
	*/
	std::string getPath( std::string const & path );
	/**
	*\~french
	*\brief
	*	Récupère le nom de fichier du chemin donné.
	*\param[in] path
	*	Le chemin.
	*\return
	*	Le nom de fichier.
	*\~english
	*\brief
	*	Retrieves the given path's file name.
	*\param[in] path
	*	The path.
	*\return
	*	The file name.
	*/
	std::string getFileName( std::string const & path );
}
/**
*\~french
*\brief
*	Concatène deux chemins en utilisant le séparateur de dossier.
*\param[in] lhs, rhs
*	Les chemins à concaténer.
*\return
*	Le chemin concaténé (lhs / rhs).
*\~english
*\brief
*	Concatenates two paths, adding the folder separator.
*\param[in] lhs, rhs
*	The paths to concatenate.
*\return
*	The concatenated path (lhs / rhs).
*/
std::string operator/( std::string const & lhs, std::string const & rhs );
/**
*\~french
*\brief
*	Concatène deux chemins en utilisant le séparateur de dossier.
*\param[in] lhs, rhs
*	Les chemins à concaténer.
*\return
*	Le chemin concaténé (lhs / rhs).
*\~english
*\brief
*	Concatenates two paths, adding the folder separator.
*\param[in] lhs, rhs
*	The paths to concatenate.
*\return
*	The concatenated path (lhs / rhs).
*/
inline std::string operator/( std::string const & lhs, char const * const rhs )
{
	return lhs / std::string{ rhs };
}
/**
*\~french
*\brief
*	Concatène deux chemins en utilisant le séparateur de dossier.
*\param[in] lhs, rhs
*	Les chemins à concaténer.
*\return
*	Le chemin concaténé (lhs / rhs).
*\~english
*\brief
*	Concatenates two paths, adding the folder separator.
*\param[in] lhs, rhs
*	The paths to concatenate.
*\return
*	The concatenated path (lhs / rhs).
*/
inline std::string operator/( char const * const lhs, std::string const & rhs )
{
	return std::string{ lhs } / rhs;
}

#endif
