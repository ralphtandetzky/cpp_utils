/** @file

  @author Ralph Tandetzky
  @date 04 Nov 2013
*/

#pragma once

#include "cow_ptr.h"
#include <initializer_list>
#include <stdexcept>

namespace cu {

template <typename Key, typename T>
class CowMap
{
public:
    T read( Key key ) const
    {
        cow_ptr<Node> n = root;
        while ( n )
        {
            if ( key < n->key )
                n = n->left;
            else if ( n->key < key )
                n = n->right;
            else
                return n->data;
        }
        throw std::out_of_range( "Invalid key in CowMap::read()." );
    }

    template <typename F>
    void readAll( F && f ) const
    {
        readAllNodes( root, f );
    }

    template <typename F>
    auto modify( const Key & key, F && f )
        -> typename std::result_of<F(T&)>::type
    {
        try
        {
            modifyNode( key, root, std::forward<F>(f) );
        }
        catch ( std::out_of_range & )
        {
            throw std::out_of_range( "Invalid key in CowMap::modify()." );
        }
    }

    template <typename F>
    void modifyAll( F && f )
    {
        modifyAllNodes( root, std::forward<F>(f) );
    }

    bool insert( const Key & key, T data )
    {
        return insertNode( key, root, std::move(data) );
    }

    bool remove( const Key & key )
    {
        return removeNode( key, root );
    }

private:
    struct Node
    {
        Node( Key key, T data )
            : key(std::move(key))
            , data(std::move(data))
            , height(1)
        {
        }

        Key key;
        T data;
        int height;
        cow_ptr<Node> left, right;
    };

    cow_ptr<Node> root;

    template <typename F>
    static void readAllNodes( const cow_ptr<Node> & node, F && f )
    {
        if ( node->left )
            readAllNodes( node->left, f );
        f( node->key, node->data );
        if ( node->right )
            readAllNodes( node->right, f );
    }

    template <typename F>
    static auto modifyNode( const Key & key, cow_ptr<Node> & node, F && f )
        -> typename std::result_of<F(T&)>::type
    {
        if ( !node )
            throw std::out_of_range( "Invalid key in CowMap::modifyNode()." );

        if ( key < node.get()->key )
            return COW_MODIFY( node )
            {
                return modifyNode( key, node->left, std::forward<F>(f) );
            };
        if ( node.get()->key < key)
            return COW_MODIFY( node )
            {
                return modifyNode( key, node->right, std::forward<F>(f) );
            };
        return f( node->data );
    }

    template <typename F>
    static void modifyAllNodes( cow_ptr<Node> & node, F && f )
    {
        if ( node->left )
            modifyAllNodes( node->left, f );
        f( (const Key &)node->key, node->data );
        if ( node->right )
            modifyAllNodes( node->right, f );
    }

    static bool insertNode( const Key & key, cow_ptr<Node> & node, T && data )
    {
        if ( !node )
        {
            node = make_cow<Node>( key, std::move(data) );
            return true;
        }

        if ( key < node.get()->key )
        {
            auto result = insertNode( key, node->left, std::move(data) );
            if ( result )
                balanceNode( node );
            return result;
        }

        if ( node.get()->key < key )
        {
            auto result = insertNode( key, node->right, std::move(data) );
            if ( result )
                balanceNode( node );
            return result;
        }

        COW_MODIFY( node )
        {
            node->key = key;
            node->data = std::move(data);
        };
        return false;
    }

    static bool removeNode( const Key & key, cow_ptr<Node> & node )
    {
        if ( !node )
            return false;

        if ( key < node.get()->key )
        {
            auto result = removeNode( key, node->left );
            if ( result )
                balanceNode( node );
            return result;
        }

        if ( node.get()->key < key )
        {
            auto result = removeNode( key, node->right );
            if ( result )
                balanceNode( node );
            return result;
        }

        if ( !node.get()->left )
        {
            node = node.get()->right;
            return true;
        }

        if ( !node.get()->right )
        {
            node = node.get()->left;
            return true;
        }

        {
            auto replacement = popLeftMost( node->right );
            replacement->left  = node.get()->left ;
            replacement->right = node.get()->right;
            node = replacement;
        }
        balanceNode( node );

        return true;
    }

    static void balanceNode( cow_ptr<Node> & node )
    {

        const auto * lNode = node.get()->left .get();
        const auto * rNode = node.get()->right.get();
        const auto lHeight = lNode ? lNode->height : 0;
        const auto rHeight = rNode ? rNode->height : 0;
        const auto maxHeight = std::max( lHeight, rHeight );
        const auto minHeight = std::min( lHeight, rHeight );

        if ( maxHeight - minHeight < 2 )
        {
            if ( node.get()->height != maxHeight + 1 )
                node->height = maxHeight + 1;
            return;
        }

        assert( maxHeight - minHeight == 2 );

        if ( lHeight > rHeight )
        {
            const auto * llNode = lNode->left .get();
            const auto * lrNode = lNode->right.get();
            const auto llHeight = llNode ? llNode->height : 0;
            const auto lrHeight = lrNode ? lrNode->height : 0;
            if ( llHeight < lrHeight )
            {
                rotateLeft( node->left );
            }
            rotateRight( node );
        }
        else
        {
            const auto * rlNode = rNode->left .get();
            const auto * rrNode = rNode->right.get();
            const auto rlHeight = rlNode ? rlNode->height : 0;
            const auto rrHeight = rrNode ? rrNode->height : 0;
            if ( rrHeight < rlHeight )
            {
                rotateRight( node->right );
            }
            rotateLeft( node );
        }
    }

    static void rotateLeft( cow_ptr<Node> & node )
    {
        auto & _3 = node;
        assert( _3 );
        const auto A = _3.get()->left;
        auto _4 = _3->right;
        assert( _4 );
        const auto B = _4.get()->left;
        const auto C = _4.get()->right;
        _3->right = B;
        _3->height = std::max( A ? A->height : 0, B ? B->height : 0 ) + 1;
        _4->left = _3;
        _4->height = std::max(    _3->height    , C ? C->height : 0 ) + 1;
        node = _4;
    }

    static void rotateRight( cow_ptr<Node> & node )
    {
        auto & _5 = node;
        assert( _5 );
        const auto D = _5.get()->right;
        auto _4 = _5->left;
        assert( _4 );
        const auto B = _4.get()->left;
        const auto C = _4.get()->right;
        _5->left = C;
        _5->height = std::max( C ? C->height : 0, D ? D->height : 0 ) + 1;
        _4->right = _5;
        _4->height = std::max( B ? B->height : 0,    _5->height     ) + 1;
        node = _4;
    }

    static cow_ptr<Node> popLeftMost( cow_ptr<Node> & node )
    {
        if ( !node.get()->left )
        {
            const auto result = node;
            node = node->right;
            return result;
        }

        return popLeftMost( node->left );
    }
};

}
