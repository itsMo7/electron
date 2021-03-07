// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/browser/electron_autofill_driver.h"

#include <memory>

#include <utility>

#include "content/public/browser/render_widget_host_view.h"
#include "shell/browser/api/electron_api_web_contents.h"
#include "shell/browser/javascript_environment.h"
#include "shell/browser/native_window.h"

namespace electron {

AutofillDriver::AutofillDriver(
    content::RenderFrameHost* render_frame_host,
    mojo::PendingAssociatedReceiver<mojom::ElectronAutofillDriver> request)
    : render_frame_host_(render_frame_host),
      receiver_(this, std::move(request)) {
  autofill_popup_ = std::make_unique<AutofillPopup>();
}

AutofillDriver::~AutofillDriver() = default;

void AutofillDriver::ShowAutofillPopup(
    const gfx::RectF& bounds,
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels) {
  v8::Isolate* isolate = JavascriptEnvironment::GetIsolate();
  v8::HandleScope scope(isolate);
  auto* web_contents = api::WebContents::From(
      content::WebContents::FromRenderFrameHost(render_frame_host_));
  if (!web_contents || !web_contents->owner_window())
    return;

  auto* embedder = web_contents->embedder();

  bool osr =
      web_contents->IsOffScreen() || (embedder && embedder->IsOffScreen());
  gfx::RectF popup_bounds(bounds);
  content::RenderFrameHost* embedder_frame_host = nullptr;
  if (embedder) {
    auto* embedder_view = embedder->web_contents()->GetMainFrame()->GetView();
    auto* view = web_contents->web_contents()->GetMainFrame()->GetView();
    auto offset = view->GetViewBounds().origin() -
                  embedder_view->GetViewBounds().origin();
    popup_bounds.Offset(offset);
    embedder_frame_host = embedder->web_contents()->GetMainFrame();
  }

  autofill_popup_->CreateView(render_frame_host_, embedder_frame_host, osr,
                              web_contents->owner_window()->content_view(),
                              popup_bounds);
  autofill_popup_->SetItems(values, labels);
}

void AutofillDriver::HideAutofillPopup() {
  if (autofill_popup_)
    autofill_popup_->Hide();
}

}  // namespace electron
