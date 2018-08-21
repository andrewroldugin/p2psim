// -*- c-basic-offset: 4; related-file-name: "../../lib/bighashmap_arena.cc" -*-
#ifndef CLICK_BIGHASHMAP_ARENA_HH
#define CLICK_BIGHASHMAP_ARENA_HH

class HashMap_Arena { public:

    HashMap_Arena(unsigned element_size);

    void use()			{ _refcount++; }
    void unuse();

    bool detached() const	{ return _detached; }
    void detach()		{ _detached = true; }

    void *alloc();
    void afree(void *);
    
  private:

    struct Link {
	Link *next;
    };
    Link *_free;

    enum { NELEMENTS = 127 };	// not a power of 2 so we don't fall into a
				// too-large bucket
    char *_cur_buffer;
    int _buffer_pos;
    
    unsigned _element_size;

    char **_buffers;
    int _nbuffers;
    int _buffers_cap;

    unsigned _refcount;
    bool _detached;
    
    ~HashMap_Arena();
    void *hard_alloc();

    friend class Link;		// shut up, compiler
    
};

class HashMap_ArenaFactory { public:

    HashMap_ArenaFactory();
    virtual ~HashMap_ArenaFactory();

    static void static_initialize();
    static void static_cleanup();
    
    static HashMap_Arena *get_arena(unsigned, HashMap_ArenaFactory * =0);
    virtual HashMap_Arena *get_arena_func(unsigned);
    
  private:

    HashMap_Arena **_arenas[2];
    int _narenas[2];

    static HashMap_ArenaFactory *the_factory;
    
};

inline void
HashMap_Arena::unuse()
{
    _refcount--;
    if (_refcount <= 0)
	delete this;
}

inline void *
HashMap_Arena::alloc()
{
    if (_free) {
	void *ret = _free;
	_free = _free->next;
	return ret;
    } else if (_buffer_pos > 0) {
	_buffer_pos -= _element_size;
	return _cur_buffer + _buffer_pos;
    } else
	return hard_alloc();
}

inline void
HashMap_Arena::afree(void *v)
{
    Link *link = reinterpret_cast<Link *>(v);
    link->next = _free;
    _free = link;
}

#endif
