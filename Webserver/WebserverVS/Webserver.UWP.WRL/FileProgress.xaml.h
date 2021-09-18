//
// FileProgress.xaml.h
// Deklaration der FileProgress-Klasse
//

#pragma once

#include "FileProgress.g.h"

namespace Webserver_UWP_WRL
{
	/// <summary>
	/// Eine leere Seite, die eigenständig verwendet oder zu der innerhalb eines Rahmens navigiert werden kann.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class FileProgress sealed
	{
	private:
		int fd;
	public:
		FileProgress();
		property int FileDescriptor {
			int get() {
				return fd;
			}
			void set(int value)
			{
				fd = value;
			}
		}
		property double Progress {
			void set(double value)
			{
				FileProgressbar->Value = value;
			}
		}

		property Platform::String^ FileName
		{
			void set(Platform::String^ value)
			{
				File->Text = value;
			}
		}
	};
}