/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "sinc-frame-capture-model.h"

namespace ns3 {namespace sincgars {

NS_OBJECT_ENSURE_REGISTERED (FrameCaptureModel);

TypeId FrameCaptureModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::sinc-FrameCaptureModel")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
  ;
  return tid;
}

}
} //namespace ns3
