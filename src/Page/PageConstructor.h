#pragma once

class Page;
class Folder;

class PageConstructor
{

public:
	static Page const* ConstructPage(const Page* const page, const Folder* const root);
};