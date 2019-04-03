/* Copyright (C) 2019 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/controls/reactive_fallback.h"

namespace BT
{

void ReactiveFallback::halt()
{
    std::fill(is_asynch_child_.begin(), is_asynch_child_.end(), false );
    ControlNode::halt();
}

NodeStatus ReactiveFallback::tick()
{
    is_asynch_child_.resize(childrenCount(), false);
    size_t failure_count = 0;

    for (size_t index = 0; index < childrenCount(); index++)
    {
        TreeNode* current_child_node = children_nodes_[index];
        const NodeStatus child_status = current_child_node->executeTick();

        if( is_asynch_child_[index] &&
            current_child_node->status() == NodeStatus::FAILURE )
        {
            failure_count++;
            continue; // skip already executed asynch children
        }

        switch (child_status)
        {
            case NodeStatus::RUNNING:
            {
                is_asynch_child_[index] = true;
                haltChildren(index+1);
                return NodeStatus::RUNNING;
            }

            case NodeStatus::FAILURE:
            {
                failure_count++;
            }break;

            case NodeStatus::SUCCESS:
            {
                halt();
                return NodeStatus::SUCCESS;
            }

            case NodeStatus::IDLE:
            {
                throw LogicError("A child node must never return IDLE");
            }
        }   // end switch
    } //end for

    if( failure_count != childrenCount() )
    {
        throw LogicError("This is not supposed to happen");
    }
    halt();
    return NodeStatus::FAILURE;
}

}
